#include <bonobo/bonobo-async.h>

struct _BonoboAsyncReply {
	CORBA_Object        object;
	gpointer           *args;

	const BonoboAsyncMethod *method;

	CORBA_Environment   ev;
	CORBA_Environment  *cur_ev;

	BonoboAsyncCallback cb;
	gpointer            user_data;

	GIOPConnection     *request_cnx;

	guint               timeout_msec;
	GIOP_unsigned_long  request_id;

	GIOPRecvBuffer     *recv_buffer;
};

static BonoboAsyncReply *
handle_new (const BonoboAsyncMethod *method,
	    BonoboAsyncCallback      cb,
	    gpointer                 user_data,
	    guint                    timeout_msec,
	    CORBA_Object             object,
	    gpointer                *args,
	    CORBA_Environment       *ev)
{
	BonoboAsyncReply *handle = g_new0 (BonoboAsyncReply, 1);
	int             i, num_args = 0;
	gpointer       *src;
	BonoboAsyncArg *arg;

	for (arg = (BonoboAsyncArg *) method->arguments; arg && arg->flag; ++arg) 
		++num_args;

	handle->cb = cb;
	handle->object = CORBA_Object_duplicate (object, ev);;
	handle->method = method;
	handle->user_data = user_data;
	handle->request_cnx = ORBit_object_get_connection (object);
	giop_connection_ref (handle->request_cnx);
	handle->request_id = GPOINTER_TO_UINT (handle);
	handle->args = g_new0 (gpointer, num_args);
	handle->timeout_msec = timeout_msec;

	/*
	 * We have to copy in case of an evil forwarding
	 * later and the raving inefficiency this gives us,
	 * FIXME: should prolly just keep the marshalled buffer
	 * instead.
	 */
	src = args;
	for (i = 0; i < num_args; i++) {
		/* No point in passing inouts by reference. */
		if (method->arguments [i].flag & BONOBO_ASYNC_IN) {
			handle->args [i] = ORBit_copy_value (
				*src, method->arguments [i].type);
			src++;
		}
	}
	
	CORBA_exception_init (&handle->ev);
	handle->cur_ev = ev;

	return handle;
}

static void
got_reply (BonoboAsyncReply *handle)
{
	g_source_remove_by_user_data (handle);
}

static void
handle_free (BonoboAsyncReply *handle)
{
	int i = 0;
	CORBA_Environment ev;

	got_reply (handle);

	CORBA_exception_init (&ev);
	CORBA_Object_release (handle->object, &ev);
	CORBA_exception_free (&ev);

	while (&handle->method->arguments [i] && handle->method->arguments [i].type) {
		if (handle->args [i])
			CORBA_free (handle->args [i]);
		i++;
	}

	giop_connection_unref (handle->request_cnx);
	g_free (handle->args);
	g_free (handle);
}


/*
 * ORBit_handle_exception: hacked from env.c; we don't want our own
 * de-marshallers, use a fully generic mechanism instead.
 */
static void
demarshal_exception (BonoboAsyncReply *handle,
		     GIOPRecvBuffer   *rb)
{
	CORBA_ORB              orb = handle->object->orb;
	CORBA_SystemException *new;
	CORBA_unsigned_long     len, completion_status;
	CORBA_char             *my_repoid;
	CORBA_Environment      *ev = handle->cur_ev;
	const CORBA_TypeCode   *types = handle->method->exceptions;

	g_return_if_fail (GIOP_MESSAGE_BUFFER (rb)->message_header.message_type == GIOP_REPLY);

	rb->cur = ALIGN_ADDRESS (rb->cur, sizeof (len));
	rb->decoder (&len, rb->cur, sizeof (len));
	rb->cur = ((guchar *)rb->cur) + sizeof (len);

	if (len) {
		my_repoid = rb->cur;
		rb->cur = ((guchar *)rb->cur) + len;
	} else
		my_repoid = NULL;

	if (rb->message.u.reply.reply_status == CORBA_SYSTEM_EXCEPTION) {
		CORBA_unsigned_long minor;

		ev->_major = CORBA_SYSTEM_EXCEPTION;

		rb->cur = ALIGN_ADDRESS (rb->cur, sizeof (minor));
		rb->decoder (&minor, rb->cur, sizeof (minor));
		rb->cur = ((guchar *) rb->cur) + sizeof (minor);

		rb->cur = ALIGN_ADDRESS (rb->cur, sizeof (completion_status));
		rb->decoder (&completion_status, rb->cur, sizeof (completion_status));
		rb->cur = ((guchar *)rb->cur) + sizeof (completion_status);

		new = ORBit_alloc (sizeof (CORBA_SystemException), NULL, NULL);

		if (new) {
			new->minor = minor;
			new->completed = completion_status;
			
			/* XXX what should the repo ID be? */
			CORBA_exception_set (ev, CORBA_SYSTEM_EXCEPTION,
					     my_repoid, new);
		}
	} else if (rb->message.u.reply.reply_status == CORBA_USER_EXCEPTION) {
		int i;

		if (!types) {
			/* weirdness; they raised an exception that we don't
			   know about */
			CORBA_exception_set_system (ev, ex_CORBA_MARSHAL,
						    CORBA_COMPLETED_MAYBE);
		} else {
			for (i = 0; types [i]; i++) {
				if (!strcmp (types [i]->repo_id, my_repoid))
					break;
			}

			if (!types [i]) {
				/* weirdness; they raised an exception
				   that we don't know about */
				CORBA_exception_set_system (ev, ex_CORBA_MARSHAL,
							    CORBA_COMPLETED_MAYBE);
			} else {
				gpointer data =
					ORBit_demarshal_arg (rb, types [i], TRUE, orb);
				CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
						     types [i]->repo_id, data);
			}
		}
	};
}

static void bonobo_async_marshal (BonoboAsyncReply *handle);

static void
handle_wire_data (GIOPRecvBuffer   *recv_buffer,
		  BonoboAsyncReply *handle)
{
	got_reply (handle);

	handle->recv_buffer = recv_buffer;

	if (recv_buffer->message.u.reply.reply_status != GIOP_NO_EXCEPTION) {
		if (recv_buffer->message.u.reply.reply_status == GIOP_LOCATION_FORWARD) {

			if (handle->object->forward_locations != NULL)
				ORBit_delete_profiles (handle->object->forward_locations);

			handle->object->forward_locations = ORBit_demarshal_IOR (recv_buffer);
			giop_recv_buffer_unuse (recv_buffer);

			giop_connection_unref (handle->request_cnx);
			handle->request_cnx =
				ORBit_object_get_forwarded_connection (handle->object);

			bonobo_async_marshal (handle);
			return;
		} else {
			demarshal_exception (handle, recv_buffer);
			giop_recv_buffer_unuse (recv_buffer);
		}
	}
}

static void
timeout_fn (BonoboAsyncReply *handle)
{
	got_reply (handle);

	CORBA_exception_set_system (handle->cur_ev,
				    ex_CORBA_COMM_FAILURE,
				    CORBA_COMPLETED_NO);

	handle->cb (handle, handle->cur_ev, handle->user_data);

	handle_free (handle);
}

static gboolean
idle_fn (BonoboAsyncReply *handle)
{
	GIOPRecvBuffer *recv_buffer;

	if (!(recv_buffer =
	      giop_recv_reply_buffer_use_2 (
		      handle->request_cnx,
		      handle->request_id, FALSE)))
		return TRUE;

	handle_wire_data (recv_buffer, handle);

	handle->cb (handle, handle->cur_ev, handle->user_data);
	
	handle_free (handle);

	return FALSE;
}

typedef struct {
	CORBA_unsigned_long len;
	char                opname[1];
} OpData;

static void
bonobo_async_marshal (BonoboAsyncReply *handle)
{
	int             namelen, i = 0;
	OpData         *data;
	struct iovec    vector;
	GIOPSendBuffer *send_buffer;

	namelen = strlen (handle->method->name);
	data = g_malloc (sizeof (OpData) + namelen);
	data->len = namelen + 1;
	memcpy (&data->opname, handle->method->name, namelen + 1);
	
	vector.iov_base = data;
	vector.iov_len  = data->len + sizeof (CORBA_unsigned_long);

/*	g_warning ("Sending '%s' with id 0x%x ", ((guchar *)data) + 4,
	handle->request_id);*/
					       
	send_buffer = giop_send_request_buffer_use (
		handle->request_cnx, NULL, handle->request_id, CORBA_TRUE,
		&(handle->object->active_profile->object_key_vec),
		&vector, &ORBit_default_principal_iovec);

	if (send_buffer) {
		while (&handle->method->arguments [i] && handle->method->arguments [i].type) {
			if (handle->method->arguments [i].flag & BONOBO_ASYNC_IN)
				ORBit_marshal_arg (send_buffer, 
						   handle->args [i],
						   handle->method->arguments [i].type);
			i++;
		}

		g_assert (handle->request_cnx->fd > 0);

		/* We add the timeout function. */
		g_timeout_add (handle->timeout_msec,
			       (GSourceFunc) timeout_fn, handle);
			
		/*
		 * an old and broken scheme
		 *
		g_io_add_watch (handle->channel,
				G_IO_ERR | G_IO_HUP | G_IO_NVAL,
				(GIOFunc) demarshal_err_fn, handle);*/

		g_idle_add ((GSourceFunc) idle_fn, handle);

		giop_send_buffer_write (send_buffer);
		giop_send_buffer_unuse (send_buffer);

		handle->cur_ev = &handle->ev;
	} else {
		handle_free (handle);
		CORBA_exception_set_system (handle->cur_ev,
					    ex_CORBA_COMM_FAILURE,
					    CORBA_COMPLETED_NO);
	}
	g_free (data);
}

/**
 * bonobo_async_demarshal:
 * @handle: the reply handle
 * @retval: pointer to variable to store retval in
 * @out_args: array of pointers to vars for InOut / Out params.
 * 
 * When a user's async callback happens ( when a method's return data
 * arrives back ) this function is typicaly used by the callback
 * to de-marshal the arguments associated with the method, such as
 * the @retval, and an array of pointers to memory in which to store
 * the returned InOut / Out arguments in order.
 **/
void
bonobo_async_demarshal (BonoboAsyncReply *handle,
			gpointer          retval,
			gpointer         *out_args)
{
	GIOPRecvBuffer *recv_buffer;
	gpointer        value, src;
	CORBA_TypeCode  tc;
	CORBA_ORB       orb;
	int             i = 0;
       
	g_return_if_fail (handle != NULL);
	g_return_if_fail (retval != NULL);

	recv_buffer = handle->recv_buffer;

	if (recv_buffer->message.u.reply.reply_status != GIOP_NO_EXCEPTION) {
		g_warning ("Trying to demarshal args, when exception was fired");
		return;
	}

	orb = handle->object->orb;

	/* This is sluggish because ORBit_demarshal_value is not public */
	tc = handle->method->ret_type;
	src = value = ORBit_demarshal_arg (recv_buffer, tc, TRUE, orb);
	_ORBit_copy_value (&src, &retval, tc);
	CORBA_free (value);

	while (&handle->method->arguments [i] && handle->method->arguments [i].type) {
		tc = handle->method->arguments [i].type;

		/* This is sluggish because ORBit_demarshal_value is not public */
		if (handle->method->arguments [i].flag & BONOBO_ASYNC_OUT) {
			gpointer dest;

			g_return_if_fail (out_args != NULL);

			dest = *out_args++;
			src = value = ORBit_demarshal_arg (recv_buffer, tc, TRUE, orb);
			_ORBit_copy_value (&src, &dest, tc);
			CORBA_free (value);
		}
	}
}

/**
 * bonobo_async_invoke:
 * @method: method description
 * @cb: async callback
 * @user_data: associated callback user data
 * @timeout_msec: timeout in milli-seconds
 * @object: object to invoke method on
 * @args: array of ordered In/InOut arguments in same order as in method
 * @ev: CORBA exception environment
 * 
 * Given a type based description of a CORBA @method
 * the method is invoked asynchronously. If an error occurs
 * in invocation an exception is fired in @ev and @cb will
 * never be invoked.
 * Otherwise, @cb is invoked, either on the timeout - in
 * which case an ex_CORBA_COMM_FAILURE system exception will be
 * returned, or when the method returns with whatever data /
 * exceptions are relevant.
 **/
void
bonobo_async_invoke (const BonoboAsyncMethod *method,
		     BonoboAsyncCallback      cb,
		     gpointer                 user_data,
		     guint                    timeout_msec,
		     CORBA_Object             object,
		     gpointer                *args,
		     CORBA_Environment       *ev)
{
	BonoboAsyncReply *handle = handle_new (method, cb, user_data,
					       timeout_msec,
					       object, args, ev);

	bonobo_async_marshal (handle);
}

/**
 * bonobo_async_handle_get_recv:
 * @handle: the reply handle.
 * 
 * This method can be used to extract the internal GIOP
 * buffer on the reply, for advanced custom de-marshaling.
 * 
 * Return value: the internal reply buffer.
 **/
GIOPRecvBuffer *
bonobo_async_handle_get_recv (BonoboAsyncReply *handle)
{
	g_return_val_if_fail (handle != NULL, NULL);

	return handle->recv_buffer;
}
