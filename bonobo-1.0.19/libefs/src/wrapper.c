/* wrapper.c - file and directory wrapper functions

   Copyright (C) 2000 Maurer IT Systemlösungen KEG

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Dietmar Maurer <dietmar@maurer-it.com>

*/

#include <string.h>
#include "efs_internal.h"

#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif

static gint
gzstream_get_byte (EFSFile *file)
{
	GZStream *s = file->gzstream;
       
	if (s->eof) return EOF;
	if (s->stream.avail_in == 0) {
		gint32 res;
		res = file->efs->driver->fops->
			file_read (file, s->inbuf, Z_BUFSIZE,
				   &s->stream.avail_in);
		if (!res && !s->stream.avail_in) s->eof = 1;
		else if (res) { s->err = Z_ERRNO; return EOF; }

		s->stream.next_in = s->inbuf;
	}
	s->stream.avail_in--;
	return *(s->stream.next_in)++;
}

static void 
gzstream_put_long (EFSFile *file, guint32 x)
{
	x = GUINT32_TO_LE(x);
	file->efs->driver->fops->file_write (file, &x, 4);
}

static guint32 
gzstream_get_long (EFSFile *file)
{
	guint32 x = (guint32)gzstream_get_byte(file);
	int c;

	x += ((guint32)gzstream_get_byte(file))<<8;
	x += ((guint32)gzstream_get_byte(file))<<16;
	c = gzstream_get_byte(file);
	if (c == EOF) file->gzstream->err = Z_DATA_ERROR;
	x += ((guint32)c)<<24;
	return x;
}

static gint
gzstream_flush (EFSFile *file, gint flush)
{
	GZStream *s = file->gzstream;
	guint len;
	gint done = 0;

	if (!(file->mode&EFS_WRITE)) return Z_STREAM_ERROR;

	s->stream.avail_in = 0; /* should be zero already anyway */

	for (;;) {
		len = Z_BUFSIZE - s->stream.avail_out;
		if (len != 0) {
			if (file->efs->driver->fops->
			    file_write (file, s->outbuf, len))
				return s->err = Z_ERRNO;
			s->stream.next_out = s->outbuf;
			s->stream.avail_out = Z_BUFSIZE;
		}
		if (done) break;
		s->err = deflate(&(s->stream), flush);

		/* Ignore the second of two consecutive flushes: */
		if (len == 0 && s->err == Z_BUF_ERROR) s->err = Z_OK;

		/* deflate has finished flushing only when it hasn't used up
		 * all the available space in the output buffer: 
		 */
		done = (s->stream.avail_out != 0 || s->err == Z_STREAM_END);
 
		if (s->err != Z_OK && s->err != Z_STREAM_END) break;
	}
	return  s->err == Z_STREAM_END ? Z_OK : s->err;
}

static void
efs_destroy_file (EFSFile *file)
{
	if (!file) return;
	
	if (file->gzstream) {
		if (file->gzstream->stream.state != NULL) {
			if (file->mode&EFS_WRITE) {
				deflateEnd(&(file->gzstream->stream));
			} else {
				inflateEnd(&(file->gzstream->stream));
			}
		}

		if (file->gzstream->inbuf) g_free (file->gzstream->inbuf);
		if (file->gzstream->outbuf) g_free (file->gzstream->outbuf);
		if (file->gzstream) g_free (file->gzstream);
	}

	g_free (file);
}

static gint
gzstream_rewind (EFSFile *file)
{
	GZStream *s = file->gzstream;
	gint32 p;

	s->err = Z_OK;
	s->eof = 0;
	s->stream.avail_in = 0;
	s->stream.next_in = s->inbuf;
	s->crc = crc32(0L, Z_NULL, 0);
	inflateReset(&s->stream);
	return file->efs->driver->fops->file_seek (file, 0, EFS_SEEK_SET, &p);
}

/**
 * efs_node_open:
 * @node: return value
 * @parent: parent directory
 * @path: filesystem path
 * @flags: access flags
 * @type: prefered node type (EFS_FILE or EFS_DIR or zero)
 *
 * Description: Opens or creates a new node. @path
 * is simply the name of the node like "test.c". "./test.c" is not valid
 * and will always return NULL.
 *
 * @flags is %EFS_READ or %EFS_RDWR which request opening the 
 * #EFSFile read-only or read/write. flags  may  also  be  bitwise-or'd 
 * with one or more of the following:
 *
 * %EFS_CREATE If the #EFSFile does not exist it will be created.
 *
 * %EFS_EXCL When used with %EFS_CREATE, if the #EFSFile already 
 * exists it is an error and the open will fail.
 *
 * %EFS_COMP Creates a compressed file when used with %EFS_CREATE. Compressed
 * files are not seekable. 
 *
 * %EFS_APPEND The file  is  opened  in  append mode. Before each write, the 
 * file pointer is positioned at the end of the  file.
 *
 * Returns:
 */

EFSResult
efs_node_open (EFSNode **node, EFSDir *parent, const char *path, 
	       gint flags, gint type)
{
	EFSResult res;
	
	g_return_val_if_fail (node != NULL, EFS_ERR_INVAL);
	*node = NULL;
	g_return_val_if_fail (parent != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (parent->mode & EFS_DIR, EFS_ERR_INVAL);
	g_return_val_if_fail (path != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (!(flags&(~(EFS_RDWR|EFS_CREATE|EFS_COMP|
					 EFS_EXCL|EFS_APPEND))),
			      EFS_ERR_INVAL);
	g_return_val_if_fail (!((flags&EFS_COMP)&&(flags&EFS_APPEND)),
			      EFS_ERR_INVAL);
	g_return_val_if_fail (!(type&(~(EFS_FILE|EFS_DIR))),
			      EFS_ERR_INVAL);

	if (!(parent->mode&EFS_WRITE)) flags &= ~(EFS_CREATE|EFS_WRITE); 

	if (flags&EFS_CREATE) flags |= EFS_WRITE;
	if (flags&EFS_WRITE) flags |= EFS_READ;
	if (!(flags&EFS_RDWR)) flags |= EFS_READ;

	if  ((flags&EFS_WRITE)&&(!(parent->efs->mode&EFS_WRITE)))  
	     return EFS_ERR_PERM;
	
	res = parent->efs->driver->fops->node_open (node, parent, path, flags, 
						 type);
	
	if (res) return res;
 
	if (!(*node)) return EFS_ERR_INT;
	if (type & EFS_DIR) return EFS_ERR_OK;

	if ((*node)->mode&EFS_COMP) {
		(*node)->gzstream = g_new0 (GZStream, 1);
		(*node)->gzstream->err = Z_OK;
		(*node)->gzstream->crc = crc32(0L, Z_NULL, 0);

		if ((*node)->mode&EFS_WRITE) {
			if (deflateInit2(&((*node)->gzstream->stream),
					 Z_DEFAULT_COMPRESSION,
					 Z_DEFLATED, -MAX_WBITS, 
					 DEF_MEM_LEVEL,
					 Z_DEFAULT_STRATEGY) != Z_OK) {
				efs_destroy_file (*node);
				return EFS_ERR_INT;
			}
			(*node)->gzstream->stream.next_out = 
				(*node)->gzstream->outbuf = 
				g_malloc (Z_BUFSIZE);
		} else { 
			(*node)->gzstream->stream.next_in = 
				(*node)->gzstream->inbuf = 
				g_malloc (Z_BUFSIZE);
			if (inflateInit2(&((*node)->gzstream->stream),
					 -MAX_WBITS) != Z_OK) {
				efs_destroy_file (*node);
				return EFS_ERR_INT;
			}
		}
		(*node)->gzstream->stream.avail_out = Z_BUFSIZE;
	}
	
	return EFS_ERR_OK;
}

/**
 * efs_file_close
 * @file: file descriptor
 *
 * Description: Closes the file.
 *
 * Returns: 0 upon success. -1 upon failure.
 */

EFSResult
efs_file_close (EFSFile *file)
{
	EFSResult res;

	g_return_val_if_fail (file != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (file->mode&EFS_FILE, EFS_ERR_INVAL);

	if ((file->mode&EFS_COMP)&&(file->mode&EFS_WRITE)) {
		GZStream *s = file->gzstream;

		if (gzstream_flush (file, Z_FINISH) != Z_OK) {
			file->efs->driver->fops->node_close (file);
			efs_destroy_file (file);
			return EFS_ERR_INT;
		}
		gzstream_put_long (file, s->crc);
	}

	res = file->efs->driver->fops->node_close (file);

	efs_destroy_file (file);
	return res;
}

/**
 * efs_file_seek
 * @file: file descriptor
 * @offset:
 * @whence:
 * 
 * Description: This function repositions the offset of the @file
 * to the argument @offset according to the directive whence as follows:
 *
 * %EFS_SEEK_SET The offset is set to offset bytes.
 *
 * %EFS_SEEK_CUR The offset is set to its current location plus offset bytes.
 *
 * %EFS_SEEK_END The offset is set to the size of the file plus offset bytes.
 *
 * Returns: Upon  successful  completion,  lseek returns the resulting
 * offset location as measured in bytes from the beginning of the file.  
 * Otherwise, a value of -1 is returned. 
 */

EFSResult
efs_file_seek (EFSFile *file, gint32 offset, gint whence, guint32 *pos)
{
	g_return_val_if_fail (file != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (file->mode&EFS_FILE, EFS_ERR_INVAL);

	if (pos) *pos = 0;

	if (file->mode&EFS_COMP) {
		if (!((offset==0)&&(whence==EFS_SEEK_SET))) 
			return EFS_ERR_NOSEEK;
		if (file->mode&EFS_WRITE) return EFS_ERR_NOSEEK;
		return gzstream_rewind (file);
	} else {
		guint32 *p, tmp_pos;

		if (pos) p = pos; else p = &tmp_pos;

		return file->efs->driver->fops->
			file_seek (file, offset, whence, p);
	}
}

/**
 * efs_file_tell
 * @file: file descriptor
 * @pos: return value
 * 
 * Description: This function obtains the current value of the file
 * position indicator for @file.
 *
 * Returns: the current position indicator, or -1 on error.
 */

EFSResult
efs_file_tell (EFSFile *file, guint32 *pos)
{
	g_return_val_if_fail (file != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (file->mode&EFS_FILE, EFS_ERR_INVAL);
	g_return_val_if_fail (pos != NULL, EFS_ERR_INVAL);

	if (file->mode&EFS_COMP) {
		if (file->mode&EFS_WRITE) 
			*pos = file->gzstream->stream.total_in;
		else 
			*pos = file->gzstream->stream.total_out;
	} else *pos = file->pos;

	return EFS_ERR_OK;
}

/**
 * efs_file_read
 * @file: file descriptor
 * @buf: pointer to a buffer to store the result
 * @count: bytes to read
 *
 * Description: This function attempts  to  read  up  to  @count  bytes 
 * from @file into the buffer starting at @buf. If @count is zero it returns 
 * zero  and  has  no  other results.
 *
 * Returns: On success, the number of bytes read  is  returned  (zero
 * indicates  end of file), and the file position is advanced by this number.
 * On error, -1 is returned.
 */

EFSResult
efs_file_read (EFSFile *file, gpointer buf, gint32 count, guint32 *bytes_read)
{
	gint32 br;
	
	*bytes_read = 0;

	g_return_val_if_fail (file != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (file->mode&EFS_FILE, EFS_ERR_INVAL);
	g_return_val_if_fail (buf != NULL, EFS_ERR_INVAL);

	if (!count) return EFS_ERR_OK;

	if (file->mode&EFS_COMP) {
		EFSResult res;
		gpointer next_out;
		gpointer start;
		GZStream *s = file->gzstream;

		s->stream.next_out = start = next_out = buf;
		s->stream.avail_out = count;

		while (s->stream.avail_out != 0) {
			if (s->stream.avail_in == 0 && !s->eof) {
				if (!(res = file->efs->driver->fops->
				      file_read(file,s->inbuf,Z_BUFSIZE,&br))){
					s->stream.avail_in = br;
					if (!br) s->eof = 1;
				} else {
					s->err = Z_ERRNO; break;
				}

				s->stream.next_in = s->inbuf;
			}
			
			s->err = inflate(&(s->stream), Z_NO_FLUSH);
			if (s->err == Z_STREAM_END) {
				/* Check CRC */
				s->crc = crc32(s->crc, start, (uInt)
					       ((gpointer)s->stream.next_out -
						start));
				start = s->stream.next_out;

				if (gzstream_get_long(file) != s->crc)
					s->err = Z_DATA_ERROR;
			}
			if (s->err != Z_OK || s->eof) break;
		}
		s->crc = crc32(s->crc, start, (uInt)
			       ((gpointer)s->stream.next_out - start));

		*bytes_read = count - s->stream.avail_out;
		
		return EFS_ERR_OK;
	} else 
		return file->efs->driver->fops->
			file_read (file, buf, count, bytes_read);
}

/**
 * efs_file_write
 * @file: file descriptor
 * @buf: pointer to the data
 * @count: bytes to write
 *
 * Description: Writes up to @count bytes to @file from the buffer starting  
 * at @buf.
 *
 * Returns: On success, the number of bytes written are returned (zero
 * indicates nothing was written). On error, -1 is returned.
 */

EFSResult
efs_file_write (EFSFile *file, gpointer buf, gint32 count)
{
	
	g_return_val_if_fail (file != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (file->mode&EFS_FILE, EFS_ERR_INVAL);
	g_return_val_if_fail (buf != NULL, EFS_ERR_INVAL);

	if (!(file->mode&EFS_WRITE)) return EFS_ERR_PERM;
	if (!(file->efs->mode&EFS_WRITE)) return EFS_ERR_PERM;
	
	if (!count) return EFS_ERR_OK;
 
	if (file->mode&EFS_COMP) {
		EFSResult res = EFS_ERR_OK;

		GZStream *s = file->gzstream;
		s->stream.next_in = buf;
		s->stream.avail_in = count;

		while (s->stream.avail_in != 0) {
			if (s->stream.avail_out == 0) {
				s->stream.next_out = s->outbuf;
				if ((res = file->efs->driver->fops->
				     file_write(file, s->outbuf, 
						Z_BUFSIZE))) {
					s->err = Z_ERRNO;
					break;
				}
				s->stream.avail_out = Z_BUFSIZE;
			}
			s->err = deflate(&(s->stream), Z_NO_FLUSH);
			if (s->err != Z_OK) break;
		}
		s->crc = crc32(s->crc, (const Bytef *)buf, count);

		if (s->stream.avail_in) return EFS_ERR_INT;

		return EFS_ERR_OK;
	} else 
		return file->efs->driver->fops->file_write (file, buf, count);
}

/**
 * efs_file_trunc
 * @file: file descriptor
 * @size: new size
 *
 * Description: Causes the @file to be truncated at @size bytes.
 * If the file previously was larger than this size, the extra data
 * is  lost. If the file previously was shorter, the file is left
 * unchanged.
 *
 * Returns: On success the new file size is returned. On error, -1 is returned.
 */

EFSResult
efs_file_trunc (EFSFile *file, guint32 size)
{
	EFSResult res;

	g_return_val_if_fail (file != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (file->mode&EFS_FILE, EFS_ERR_INVAL);
	
	if (!(file->mode&EFS_WRITE)) return EFS_ERR_PERM;
	if (!(file->efs->mode&EFS_WRITE)) return EFS_ERR_PERM;
	
	if (file->mode&EFS_COMP) {
		if (size) return EFS_ERR_NOSEEK;
		if ((res = file->efs->driver->fops->file_trunc (file, 0)))
			return res;
		return gzstream_rewind (file);
	} else 
		return file->efs->driver->fops->file_trunc (file, size);
}

/**
 * efs_type_set
 * @file: file descriptor
 * @type: 
 *
 * Description: Set the type code of @file
 *
 * Returns: 
 */

EFSResult
efs_type_set (EFSNode *node, guint32 type)
{
	g_return_val_if_fail (node != NULL, EFS_ERR_INVAL);
	
	if (!(node->mode&EFS_WRITE)) return EFS_ERR_PERM;
	if (!(node->efs->mode&EFS_WRITE)) return EFS_ERR_PERM;

	if (node->mode&EFS_ROOT) {
		node->efs->type = type;
		return EFS_ERR_OK;
	} else {
		return node->efs->driver->fops->type_set (node, type);
	}
}

/**
 * efs_type_get
 * @file: file descriptor
 * @type: return value
 *
 * Description: Get the type code of @file
 *
 * Returns: 
 */

EFSResult
efs_type_get (EFSNode *node, guint32 *type)
{
	g_return_val_if_fail (node != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (type != NULL, EFS_ERR_INVAL);

	if (node->mode&EFS_ROOT) {
		*type = node->efs->type;
		return EFS_ERR_OK;
	} else {
		return node->efs->driver->fops->type_get (node, type);
	}
}


/**
 * efs_stat
 * @dir: reference to a #EFS directory
 * @path: file or directory name
 * @stat: buffer to store the result
 *
 * Description: This  functions  return  information  about the specified
 * file or directory. The result is stored in the @stat buffer.
 *
 * Returns: On success, zero is returned. On error, -1 is returned.
 */

EFSResult         
efs_stat (EFSDir *dir, const char *path, EFSStat *stat)
{
	EFSNode *node;
	EFSResult res;

	g_return_val_if_fail (dir != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (dir->mode&EFS_DIR, EFS_ERR_INVAL);
	g_return_val_if_fail (path != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (stat != NULL, EFS_ERR_INVAL);

	if ((res =  dir->efs->driver->fops->node_open 
	     (&node, dir, path, EFS_READ, 0))) return res;
	
	res = dir->efs->driver->fops->node_stat (node, stat);
	dir->efs->driver->fops->node_close (node);

	return res;
}

/**
 * efs_node_stat
 * @node: reference to a file or directory
 * @stat: buffer to store the result
 *
 * Description: This  functions  return  information  about the specified
 * file or directory. The result is stored in the @stat buffer.
 *
 * Returns:
 */

EFSResult    
efs_node_stat (EFSNode *node, EFSStat *stat)
{
	g_return_val_if_fail (node != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (stat != NULL, EFS_ERR_INVAL);

	return node->efs->driver->fops->node_stat (node, stat);
}

/**
 * efs_erase:
 * @dir: reference to a #EFS directory
 * @path: file or directory name
 *
 * Description: Delete the file/directory it refers to.
 *
 * Returns: zero on success, or -1 if an error occurred.
 */

EFSResult
efs_erase (EFSDir *dir, const char *path)
{
	g_return_val_if_fail (dir != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (dir->mode&EFS_DIR, EFS_ERR_INVAL);
	g_return_val_if_fail (path != NULL, EFS_ERR_INVAL);

	if (!(dir->mode&EFS_WRITE)) return EFS_ERR_PERM;
	if (!(dir->efs->mode&EFS_WRITE)) return EFS_ERR_PERM;
	
	return dir->efs->driver->fops->erase (dir, path);
}

/**
 * efs_dir_close
 * @dir: the directory to close.
 * 
 * Description: Closes @dir. This function 
 * recurses its opened files/subdirs. This 
 * function can be called on the root #EFS
 * after having called efs_commit.
 *
 * Returns: 0 upon success. -1 upon failure.
 */

EFSResult         
efs_dir_close (EFSDir *dir)
{
	g_return_val_if_fail (dir != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (dir->mode&EFS_DIR, EFS_ERR_INVAL);
	g_return_val_if_fail (!(dir->mode&EFS_ROOT), EFS_ERR_INVAL);

	return dir->efs->driver->fops->node_close (dir);
}

/**
 * efs_dir_seek
 * @dir: the directory to seek
 * @offset:
 *
 * Description: This function sets the location in the  directory
 * stream  from  which  the  next  efs_dir_read() call will start.
 * efs_dir_seek() should  be  used  with  an  offset  returned  by
 * efs_dir_tell() or zero.
 *
 * Returns: 0 upon success. -1 upon failure.
 */

EFSResult 
efs_dir_seek (EFSDir *dir, guint32 offset)
{
	g_return_val_if_fail (dir != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (dir->mode&EFS_DIR, EFS_ERR_INVAL);

	return dir->efs->driver->fops->dir_seek (dir, offset);
}

/**
 * efs_dir_tell
 * @dir: directory descriptor
 * @pos: return value
 * 
 * Description: This function obtains the current value of the file
 * position indicator for @dir.
 *
 * Returns: the current position indicator, or -1 on error.
 */

EFSResult
efs_dir_tell (EFSDir *dir, guint32 *pos)
{
	g_return_val_if_fail (dir != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (dir->mode&EFS_DIR, EFS_ERR_INVAL);
	g_return_val_if_fail (pos != NULL, EFS_ERR_INVAL);

	*pos = dir->pos;

	return EFS_ERR_OK;
}

/**
 * efs_dir_read
 * @dir: the directory to close.
 * @de: a place to store the result
 *
 * Description: allows you to browse @dir content.
 *
 * Returns: it will return a positive value and store the next 
 * #EFSDirEntry in @de till all of them have been returned. It will 
 * return zero after the last #EFSDirEntry has been returned, or -1 on error.
 */

EFSResult
efs_dir_read (EFSDir *dir,  EFSDirEntry *de)
{
	g_return_val_if_fail (dir != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (dir->mode&EFS_DIR, EFS_ERR_INVAL);
	g_return_val_if_fail (de != NULL, EFS_ERR_INVAL);
	
	return dir->efs->driver->fops->dir_read (dir, de);
}

/**
 * efs_rename:
 * @dir: reference to a #EFS directory
 * @old_path: file or directory name
 * @new_path: new name
 *
 * Description: Rename a file or directory.
 *
 * Returns: zero on success, or -1 if an error occurred.
 */

EFSResult
efs_rename (EFSDir *dir, const char *old_path, const char *new_path)
{
	g_return_val_if_fail (dir != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (dir->mode&EFS_DIR, EFS_ERR_INVAL);
	g_return_val_if_fail (old_path != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (new_path != NULL, EFS_ERR_INVAL);

	if (!(dir->mode&EFS_WRITE)) return EFS_ERR_PERM;
	if (!(dir->efs->mode&EFS_WRITE)) return EFS_ERR_PERM;
	
	if (!strcmp (old_path, new_path)) return EFS_ERR_OK;

	return dir->efs->driver->fops->rename (dir, old_path, new_path);
}


/**
 * efs_node_equal:
 * @node1: reference to a node
 * @node2: reference to a node
 *
 * Description: Check if the nodes are equal.
 *
 * Returns: TRUE if the nodes are equal, FALSE otherwise.
 */

gboolean
efs_node_equal (EFSNode *node1, EFSNode *node2)
{
	g_return_val_if_fail (node1 != NULL, FALSE);
	g_return_val_if_fail (node2 != NULL, FALSE);

	if (node1->efs != node2->efs) return FALSE;
	
	return node1->efs->driver->fops->node_equal (node1, node2);
}


