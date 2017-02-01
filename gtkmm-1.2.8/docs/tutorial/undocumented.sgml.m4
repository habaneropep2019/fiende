<sect> Undocumented Widgets
<!-- ***************************************************************** -->
<p>
These all require authors! :) Please consider contributing to our
tutorial.

If you must use one of these widgets that are undocumented, I strongly
suggest you take a look at their respective header files in the Gtk--
distribution. Gtk--'s function names are very descriptive. Once you
have an understanding of how things work, it's not difficult to figure
out how to use a widget simply by looking at its function
declarations. This, along with a few examples from others' code, and
it should be no problem.

When you do come to understand all the functions of a new undocumented
widget, please consider writing a tutorial on it so others may benefit
from your time.

<!-- ----------------------------------------------------------------- -->
<sect1> Calendar
<p>
<!-- ----------------------------------------------------------------- -->
<sect1> CTree
<p>
<!-- ----------------------------------------------------------------- -->
<sect1> Curves
<p>
<!-- ----------------------------------------------------------------- -->
<sect1> Drawing Area
<p>
<!-- ----------------------------------------------------------------- -->
<sect1> Font Selection Dialog
<p>
<!-- ----------------------------------------------------------------- -->
<sect1> Gamma Curve
<p>
<!-- ----------------------------------------------------------------- -->
<sect1> Image
<p>
<!-- ----------------------------------------------------------------- -->
<sect1> Packer
<p>
<!-- ----------------------------------------------------------------- -->
<sect1> Plugs and Sockets
<p>
<!-- ----------------------------------------------------------------- -->
<sect1> Preview
<p>

<!--

(This may need to be rewritten to follow the style of the rest of the tutorial)

<tscreen><verb>

Previews serve a number of purposes in GIMP/Gtk--. The most important one is
this. High quality images may take up to tens of megabytes of memory - easy!
Any operation on an image that big is bound to take a long time. If it takes
you 5-10 trial-and-errors (i.e. 10-20 steps, since you have to revert after
you make an error) to choose the desired modification, it make take you
literally hours to make the right one - if you don't run out of memory
first. People who have spent hours in color darkrooms know the feeling.
Previews to the rescue!

But the annoyance of the delay is not the only issue. Oftentimes it is
helpful to compare the Before and After versions side-by-side or at least
back-to-back. If you're working with big images and 10 second delays,
obtaining the Before and After impressions is, to say the least, difficult.
For 30M images (4"x6", 600dpi, 24 bit) the side-by-side comparison is right
out for most people, while back-to-back is more like back-to-1001, 1002,
..., 1010-back! Previews to the rescue!

But there's more. Previews allow for side-by-side pre-previews. In other
words, you write a plug-in (e.g. the filterpack simulation) which would have
a number of here's-what-it-would-look-like-if-you-were-to-do-this previews.
An approach like this acts as a sort of a preview palette and is very
effective for subtle changes. Let's go previews!

There's more. For certain plug-ins real-time image-specific human
intervention maybe necessary. In the SuperNova plug-in, for example, the
user is asked to enter the coordinates of the center of the future
supernova. The easiest way to do this, really, is to present the user with a
preview and ask him to interactively select the spot. Let's go previews!

Finally, a couple of misc uses. One can use previews even when not working
with big images. For example, they are useful when rendering complicated
patterns. (Just check out the venerable Diffraction plug-in + many other
ones!) As another example, take a look at the colormap rotation plug-in
(work in progress). You can also use previews for little logos inside you
plug-ins and even for an image of yourself, The Author. Let's go previews!

When Not to Use Previews

Don't use previews for graphs, drawing etc. GDK is much faster for that. Use
previews only for rendered images!

Let's go previews!

You can stick a preview into just about anything. In a vbox, an hbox, a
table, a button, etc. But they look their best in tight frames around them.
Previews by themselves do not have borders and look flat without them. (Of
course, if the flat look is what you want...) Tight frames provide the
necessary borders.

                               [Image][Image]

Previews in many ways are like any other widgets in Gtk-- (whatever that
means) except they possess an additional feature: they need to be filled with
some sort of an image! First, we will deal exclusively with the Gtk-- aspect
of previews and then we'll discuss how to fill them.

GtkWidget *preview!

Without any ado:

                              /* Create a preview widget,
                              set its size, an show it */
GtkWidget *preview;
preview=gtk_preview_new(GTK_PREVIEW_COLOR)
                              /*Other option:
                              GTK_PREVIEW_GRAYSCALE);*/
gtk_preview_size (GTK_PREVIEW (preview), WIDTH, HEIGHT);
gtk_widget_show(preview);
my_preview_rendering_function(preview);

Oh yeah, like I said, previews look good inside frames, so how about:

GtkWidget *create_a_preview(int        Width,
                            int        Height,
                            int        Colorfulness)
{
  GtkWidget *preview;
  GtkWidget *frame;
  
  frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_set_border_width (GTK_CONTAINER(frame),0);
  gtk_widget_show(frame);

  preview=gtk_preview_new (Colorfulness?GTK_PREVIEW_COLOR
                                       :GTK_PREVIEW_GRAYSCALE);
  gtk_preview_size (GTK_PREVIEW (preview), Width, Height);
  gtk_container_add(GTK_CONTAINER(frame),preview);
  gtk_widget_show(preview);

  my_preview_rendering_function(preview);
  return frame;
}

That's my basic preview. This routine returns the "parent" frame so you can
place it somewhere else in your interface. Of course, you can pass the
parent frame to this routine as a parameter. In many situations, however,
the contents of the preview are changed continually by your application. In
this case you may want to pass a pointer to the preview to a
"create_a_preview()" and thus have control of it later.

One more important note that may one day save you a lot of time. Sometimes
it is desirable to label you preview. For example, you may label the preview
containing the original image as "Original" and the one containing the
modified image as "Less Original". It might occur to you to pack the
preview along with the appropriate label into a vbox. The unexpected caveat
is that if the label is wider than the preview (which may happen for a
variety of reasons unforseeable to you, from the dynamic decision on the
size of the preview to the size of the font) the frame expands and no longer
fits tightly over the preview. The same problem can probably arise in other
situations as well.

                                   [Image]

The solution is to place the preview and the label into a 2x1 table and by
attaching them with the following parameters (this is one possible variations
of course. The key is no GTK_FILL in the second attachment):

gtk_table_attach(GTK_TABLE(table),label,0,1,0,1,
                 0,
                 GTK_EXPAND|GTK_FILL,
                 0,0);
gtk_table_attach(GTK_TABLE(table),frame,0,1,1,2,
                 GTK_EXPAND,
                 GTK_EXPAND,
                 0,0);


And here's the result:

                                   [Image]

Misc

Making a preview clickable is achieved most easily by placing it in a
button. It also adds a nice border around the preview and you may not even
need to place it in a frame. See the Filter Pack Simulation plug-in for an
example.

This is pretty much it as far as Gtk-- is concerned.

Filling In a Preview

In order to familiarize ourselves with the basics of filling in previews,
let's create the following pattern (contrived by trial and error):

                                   [Image]

void
my_preview_rendering_function(GtkWidget     *preview)
{
#define SIZE 100
#define HALF (SIZE/2)

  guchar *row=(guchar *) malloc(3*SIZE); /* 3 bits per dot */
  gint i, j;                             /* Coordinates    */
  double r, alpha, x, y;

  if (preview==NULL) return; /* I usually add this when I want */
                             /* to avoid silly crashes. You    */
                             /* should probably make sure that */
                             /* everything has been nicely     */
                             /* initialized!                   */
  for (j=0; j < ABS(cos(2*alpha)) ) { /* Are we inside the shape?  */
                                         /* glib.h contains ABS(x).   */
        row[i*3+0] = sqrt(1-r)*255;      /* Define Red                */
        row[i*3+1] = 128;                /* Define Green              */
        row[i*3+2] = 224;                /* Define Blue               */
      }                                  /* "+0" is for alignment!    */
      else {
        row[i*3+0] = r*255;
        row[i*3+1] = ABS(sin((float)i/SIZE*2*PI))*255;
        row[i*3+2] = ABS(sin((float)j/SIZE*2*PI))*255;
      }
    }
    gtk_preview_draw_row( GTK_PREVIEW(preview),row,0,j,SIZE);
    /* Insert "row" into "preview" starting at the point with  */
    /* coordinates (0,j) first column, j_th row extending SIZE */
    /* pixels to the right */
  }

  free(row); /* save some space */
  gtk_widget_draw(preview,NULL); /* what does this do? */
  gdk_flush(); /* or this? */
}

Non-GIMP users can have probably seen enough to do a lot of things already.
For the GIMP users I have a few pointers to add.

Image Preview

It is probably wise to keep a reduced version of the image around with just
enough pixels to fill the preview. This is done by selecting every n'th
pixel where n is the ratio of the size of the image to the size of the
preview. All further operations (including filling in the previews) are then
performed on the reduced number of pixels only. The following is my
implementation of reducing the image. (Keep in mind that I've had only basic
C!)

(UNTESTED CODE ALERT!!!)

typedef struct {
  gint      width;
  gint      height;
  gint      bbp;
  guchar    *rgb;
  guchar    *mask;
} ReducedImage;

enum {
  SELECTION_ONLY,
  SELECTION_IN_CONTEXT,
  ENTIRE_IMAGE
};

ReducedImage *Reduce_The_Image(GDrawable *drawable,
                               GDrawable *mask,
                               gint LongerSize,
                               gint Selection)
{
  /* This function reduced the image down to the the selected preview size */
  /* The preview size is determine by LongerSize, i.e. the greater of the  */
  /* two dimensions. Works for RGB images only!                            */
  gint RH, RW;          /* Reduced height and reduced width                */
  gint width, height;   /* Width and Height of the area being reduced      */
  gint bytes=drawable->bpp;
  ReducedImage *temp=(ReducedImage *)malloc(sizeof(ReducedImage));

  guchar *tempRGB, *src_row, *tempmask, *src_mask_row,R,G,B;
  gint i, j, whichcol, whichrow, x1, x2, y1, y2;
  GPixelRgn srcPR, srcMask;
  gint NoSelectionMade=TRUE; /* Assume that we're dealing with the entire  */
                             /* image.                                     */

  gimp_drawable_mask_bounds (drawable->id, &amp;x1, &amp;y1, &amp;x2, &amp;y2);
  width  = x2-x1;
  height = y2-y1;
  /* If there's a SELECTION, we got its bounds!)

  if (width != drawable->width &amp;&amp; height != drawable->height)
    NoSelectionMade=FALSE;
  /* Become aware of whether the user has made an active selection   */
  /* This will become important later, when creating a reduced mask. */

  /* If we want to preview the entire image, overrule the above!  */
  /* Of course, if no selection has been made, this does nothing! */
  if (Selection==ENTIRE_IMAGE) {
    x1=0;
    x2=drawable->width;
    y1=0;
    y2=drawable->height;
  }

  /* If we want to preview a selection with some surrounding area we */
  /* have to expand it a little bit. Consider it a bit of a riddle. */
  if (Selection==SELECTION_IN_CONTEXT) {
    x1=MAX(0,                x1-width/2.0);
    x2=MIN(drawable->width,  x2+width/2.0);
    y1=MAX(0,                y1-height/2.0);
    y2=MIN(drawable->height, y2+height/2.0);
  }

  /* How we can determine the width and the height of the area being */
  /* reduced.                                                        */
  width  = x2-x1;
  height = y2-y1;

  /* The lines below determine which dimension is to be the longer   */
  /* side. The idea borrowed from the supernova plug-in. I suspect I */
  /* could've thought of it myself, but the truth must be told.      */
  /* Plagiarism stinks!                                               */
  if (width>height) {
    RW=LongerSize;
    RH=(float) height * (float) LongerSize/ (float) width;
  }
  else {
    RH=LongerSize;
    RW=(float)width * (float) LongerSize/ (float) height;
  }

  /* The entire image is stretched into a string! */
  tempRGB   = (guchar *) malloc(RW*RH*bytes);
  tempmask  = (guchar *) malloc(RW*RH);

  gimp_pixel_rgn_init (&amp;srcPR, drawable, x1, y1, width, height,
                       FALSE, FALSE);
  gimp_pixel_rgn_init (&amp;srcMask, mask, x1, y1, width, height,
                       FALSE, FALSE);

  /* Grab enough to save a row of image and a row of mask. */
  src_row       = (guchar *) malloc (width*bytes);
  src_mask_row  = (guchar *) malloc (width);

  for (i=0; i < RH; i++) {
    whichrow=(float)i*(float)height/(float)RH;
    gimp_pixel_rgn_get_row (&amp;srcPR, src_row, x1, y1+whichrow, width);
    gimp_pixel_rgn_get_row (&amp;srcMask, src_mask_row, x1, y1+whichrow, width);

    for (j=0; j < RW; j++) {
      whichcol=(float)j*(float)width/(float)RW;

      /* No selection made = each point is completely selected! */
      if (NoSelectionMade)
        tempmask[i*RW+j]=255;
      else
        tempmask[i*RW+j]=src_mask_row[whichcol];

      /* Add the row to the one long string which now contains the image! */
      tempRGB[i*RW*bytes+j*bytes+0]=src_row[whichcol*bytes+0];
      tempRGB[i*RW*bytes+j*bytes+1]=src_row[whichcol*bytes+1];
      tempRGB[i*RW*bytes+j*bytes+2]=src_row[whichcol*bytes+2];

      /* Hold on to the alpha as well */
      if (bytes==4)
        tempRGB[i*RW*bytes+j*bytes+3]=src_row[whichcol*bytes+3];
    }
  }
  temp->bpp=bytes;
  temp->width=RW;
  temp->height=RH;
  temp->rgb=tempRGB;
  temp->mask=tempmask;
  return temp;
}

The following is a preview function which used the same ReducedImage type!
Note that it uses fakes transparency (if one is present by means of
fake_transparency which is defined as follows:

gint fake_transparency(gint i, gint j)
{
  if ( ((i%20)- 10) * ((j%20)- 10)>0   )
    return 64;
  else
    return 196;
}

Now here's the preview function:

void
my_preview_render_function(GtkWidget     *preview,
                           gint          changewhat,
                           gint          changewhich)
{
  gint Inten, bytes=drawable->bpp;
  gint i, j, k;
  float partial;
  gint RW=reduced->width;
  gint RH=reduced->height;
  guchar *row=malloc(bytes*RW);;


  for (i=0; i < RH; i++) {
    for (j=0; j < RW; j++) {

      row[j*3+0] = reduced->rgb[i*RW*bytes + j*bytes + 0];
      row[j*3+1] = reduced->rgb[i*RW*bytes + j*bytes + 1];
      row[j*3+2] = reduced->rgb[i*RW*bytes + j*bytes + 2];

      if (bytes==4)
        for (k=0; k<3; k++) {
          float transp=reduced->rgb[i*RW*bytes+j*bytes+3]/255.0;
          row[3*j+k]=transp*a[3*j+k]+(1-transp)*fake_transparency(i,j);
        }
    }
    gtk_preview_draw_row( GTK_PREVIEW(preview),row,0,i,RW);
  }

  free(a);
  gtk_widget_draw(preview,NULL);
  gdk_flush();
}

Applicable Routines

guint           gtk_preview_get_type           (void);
/* No idea */
void            gtk_preview_uninit             (void);
/* No idea */
GtkWidget*      gtk_preview_new                (GtkPreviewType   type);
/* Described above */
void            gtk_preview_size               (GtkPreview      *preview,
                                                gint             width,
                                                gint             height);
/* Allows you to resize an existing preview.    */
/* Apparently there's a bug in Gtk-- which makes  */
/* this process messy. A way to clean up a mess */
/* is to manually resize the window containing  */
/* the preview after resizing the preview.      */

void            gtk_preview_put                (GtkPreview      *preview,
                                                GdkWindow       *window,
                                                GdkGC           *gc,
                                                gint             srcx,
                                                gint             srcy,
                                                gint             destx,
                                                gint             desty,
                                                gint             width,
                                                gint             height);
/* No idea */

void            gtk_preview_put_row            (GtkPreview      *preview,
                                                guchar          *src,
                                                guchar          *dest,
                                                gint             x,
                                                gint             y,
                                                gint             w);
/* No idea */

void            gtk_preview_draw_row           (GtkPreview      *preview,
                                                guchar          *data,
                                                gint             x,
                                                gint             y,
                                                gint             w);
/* Described in the text */

void            gtk_preview_set_expand         (GtkPreview      *preview,
                                                gint             expand);
/* No idea */

/* No clue for any of the below but    */
/* should be standard for most widgets */
void            gtk_preview_set_gamma          (double           gamma);
void            gtk_preview_set_color_cube     (guint            nred_shades,
                                                guint            ngreen_shades,
                                                guint            nblue_shades,
                                                guint            ngray_shades);
void            gtk_preview_set_install_cmap   (gint             install_cmap);
void            gtk_preview_set_reserved       (gint             nreserved);
GdkVisual*      gtk_preview_get_visual         (void);
GdkColormap*    gtk_preview_get_cmap           (void);
GtkPreviewInfo* gtk_preview_get_info           (void);

That's all, folks!

</verb></tscreen>

-->

<!-- ***************************************************************** -->
