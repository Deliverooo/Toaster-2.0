#include "openglhpp/opengl.hpp"

#include <glad/glad.h>

namespace gl
{
	Int loadGL()
	{
		return gladLoadGL();
	}

	#pragma region Rendering

	void clear(BufferMaskFlags mask)
	{
		glClear(mask);
	}

	void clearBufferIv(Enum buffer, Int draw_buffer, const Int *value)
	{
		glClearBufferiv(buffer, draw_buffer, value);
	}

	void clearBufferuiv(Enum buffer, Int draw_buffer, const UInt *value)
	{
		glClearBufferuiv(buffer, draw_buffer, value);
	}

	void clearBufferFv(Enum buffer, Int draw_buffer, const Float *p_value)
	{
		glClearBufferfv(buffer, draw_buffer, p_value);
	}

	void clearBufferFi(Enum buffer, Int draw_buffer, Float depth, Int stencil)
	{
		glClearBufferfi(buffer, draw_buffer, depth, stencil);
	}

	void clearNamedFramebufferIv(UInt p_framebuffer, Enum buffer, Int draw_buffer, const Int *value)
	{
		glClearNamedFramebufferiv(p_framebuffer, buffer, draw_buffer, value);
	}

	void clearNamedFramebufferuiv(UInt p_framebuffer, Enum buffer, Int draw_buffer, const UInt *value)
	{
		glClearNamedFramebufferuiv(p_framebuffer, buffer, draw_buffer, value);
	}

	void clearNamedFramebufferFv(UInt p_framebuffer, Enum buffer, Int draw_buffer, const Float *p_value)
	{
		glClearNamedFramebufferfv(p_framebuffer, buffer, draw_buffer, p_value);
	}

	void clearNamedFramebufferFi(UInt p_framebuffer, Enum buffer, Int draw_buffer, Float depth, Int stencil)
	{
		glClearNamedFramebufferfi(p_framebuffer, buffer, draw_buffer, depth, stencil);
	}

	void clearColor(Float red, Float green, Float blue, Float alpha)
	{
		glClearColor(red, green, blue, alpha);
	}

	void clearDepth(Double depth)
	{
		glClearDepth(depth);
	}

	void clearDepthF(Float depth)
	{
		glClearDepthf(depth);
	}

	void clearStencil(Int clear_value)
	{
		glClearStencil(clear_value);
	}

	void drawBuffer(Enum colour_buffer)
	{
		glDrawBuffer(colour_buffer);
	}

	void namedFramebufferDrawBuffer(UInt p_framebuffer, Enum buffer)
	{
		glNamedFramebufferDrawBuffer(p_framebuffer, buffer);
	}

	void finish()
	{
		glFinish();
	}

	void flush()
	{
		glFlush();
	}

	void readBuffer(Enum mode)
	{
		glReadBuffer(mode);
	}

	void namedFramebufferReadBuffer(UInt p_framebuffer, Enum mode)
	{
		glNamedFramebufferReadBuffer(p_framebuffer, mode);
	}

	void readPixels(Int x, Int y, Int width, Int height, Enum format, Enum type, Void *data)
	{
		glReadPixels(x, y, width, height, format, type, data);
	}

	void readnPixels(Int x, Int y, Int width, Int height, Enum format, Enum type, Int buffer_size, Void *data)
	{
		glReadnPixels(x, y, width, height, format, type, buffer_size, data);
	}

	#pragma endregion

	#pragma region Framebuffers

	void bindFramebuffer(FramebufferType p_target, UInt p_framebuffer)
	{
		glBindFramebuffer(static_cast<Enum>(p_target), p_framebuffer);
	}

	void bindRenderbuffer(FramebufferType p_target, UInt p_renderbuffer)
	{
		glBindRenderbuffer(static_cast<Enum>(p_target), p_renderbuffer);
	}

	void blitFramebuffer(Int srcX0, Int srcY0, Int srcX1, Int srcY1, Int dstX0, Int dstY0, Int dstX1, Int dstY1, BufferMaskFlags p_mask, TextureFiltering p_filter)
	{
		glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, p_mask, static_cast<Enum>(p_filter));
	}

	void blitNamedFramebuffer(UInt read_framebuffer, UInt draw_framebuffer, Int srcX0, Int srcY0, Int srcX1, Int srcY1, Int dstX0, Int dstY0, Int dstX1, Int dstY1,
							  BufferMaskFlags p_mask, TextureFiltering p_filter)
	{
		glBlitNamedFramebuffer(read_framebuffer, draw_framebuffer, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, p_mask, static_cast<Enum>(p_filter));
	}

	FramebufferStatus checkFramebufferStatus(FramebufferType p_target)
	{
		return static_cast<FramebufferStatus>(glCheckFramebufferStatus(static_cast<Enum>(p_target)));
	}

	FramebufferStatus checkNamedFramebufferStatus(UInt p_framebuffer, FramebufferType p_target)
	{
		return static_cast<FramebufferStatus>(glCheckNamedFramebufferStatus(p_framebuffer, static_cast<Enum>(p_target)));
	}

	void createFramebuffers(SizeI count, UInt *p_buffers)
	{
		glCreateFramebuffers(count, p_buffers);
	}

	void createRenderbuffers(SizeI count, UInt *p_renderbuffers)
	{
		glCreateRenderbuffers(count, p_renderbuffers);
	}

	void deleteFramebuffers(SizeI count, UInt *p_framebuffers)
	{
		glDeleteFramebuffers(count, p_framebuffers);
	}

	void deleteRenderbuffers(SizeI count, UInt *p_renderbuffers)
	{
		glDeleteRenderbuffers(count, p_renderbuffers);
	}

	void drawBuffers(SizeI count, const Enum *p_buffers)
	{
		glDrawBuffers(count, p_buffers);
	}

	void namedFramebufferDrawBuffers(UInt p_framebuffer, SizeI p_count, const Enum *p_buffers)
	{
		glNamedFramebufferDrawBuffers(p_framebuffer, p_count, p_buffers);
	}

	void framebufferParameteri(FramebufferType p_target, FramebufferParameter p_framebuffer_parameter, Int p_param)
	{
		glFramebufferParameteri(static_cast<Enum>(p_target), static_cast<Enum>(p_framebuffer_parameter), p_param);
	}

	void namedFramebufferParameteri(UInt p_framebuffer, FramebufferParameter p_framebuffer_parameter, Int p_param)
	{
		glNamedFramebufferParameteri(p_framebuffer, static_cast<Enum>(p_framebuffer_parameter), p_param);
	}

	void framebufferRenderbuffer(FramebufferType p_target, FramebufferAttachment p_attachment, RenderbufferType p_renderbuffer_target, UInt p_renderbuffer)
	{
		glFramebufferRenderbuffer(static_cast<Enum>(p_target), static_cast<Enum>(p_attachment), static_cast<Enum>(p_renderbuffer_target), p_renderbuffer);
	}

	void namedFramebufferRenderbuffer(UInt p_framebuffer, FramebufferAttachment p_attachment, RenderbufferType p_renderbuffer_target, UInt p_renderbuffer)
	{
		glNamedFramebufferRenderbuffer(p_framebuffer, static_cast<Enum>(p_attachment), static_cast<Enum>(p_renderbuffer_target), p_renderbuffer);
	}

	void framebufferTexture(FramebufferType p_target, FramebufferAttachment p_attachment, UInt texture, Int level)
	{
		glFramebufferTexture(static_cast<Enum>(p_target), static_cast<Enum>(p_attachment), texture, level);
	}

	void framebufferTexture1D(FramebufferType p_target, FramebufferAttachment p_attachment, TextureType p_tex_target, UInt texture, Int level)
	{
		glFramebufferTexture1D(static_cast<Enum>(p_target), static_cast<Enum>(p_attachment), static_cast<Enum>(p_tex_target), texture, level);
	}

	void framebufferTexture2D(FramebufferType p_target, FramebufferAttachment p_attachment, TextureType p_tex_target, UInt texture, Int level)
	{
		glFramebufferTexture2D(static_cast<Enum>(p_target), static_cast<Enum>(p_attachment), static_cast<Enum>(p_tex_target), texture, level);
	}

	void framebufferTexture3D(FramebufferType p_target, FramebufferAttachment p_attachment, TextureType p_tex_target, UInt texture, Int level, Int layer)
	{
		glFramebufferTexture3D(static_cast<Enum>(p_target), static_cast<Enum>(p_attachment), static_cast<Enum>(p_tex_target), texture, level, layer);
	}

	void namedFramebufferTexture(UInt p_framebuffer, FramebufferAttachment p_attachment, UInt texture, Int level)
	{
		glNamedFramebufferTexture(p_framebuffer, static_cast<Enum>(p_attachment), texture, level);
	}

	void framebufferTextureLayer(Enum target, FramebufferAttachment p_attachment, UInt texture, Int level, Int layer)
	{
		glFramebufferTextureLayer(target, static_cast<Enum>(p_attachment), texture, level, layer);
	}

	void namedFramebufferTextureLayer(UInt p_framebuffer, FramebufferAttachment p_attachment, UInt texture, Int level, Int layer)
	{
		glNamedFramebufferTextureLayer(p_framebuffer, static_cast<Enum>(p_attachment), texture, level, layer);
	}

	void genFramebuffers(SizeI count, UInt *p_buffers)
	{
		glGenFramebuffers(count, p_buffers);
	}

	void genRenderbuffers(SizeI count, UInt *p_renderbuffers)
	{
		glGenRenderbuffers(count, p_renderbuffers);
	}

	void generateMipmap(TextureType p_target)
	{
		glGenerateMipmap(static_cast<Enum>(p_target));
	}

	void generateTextureMipmap(UInt texture)
	{
		glGenerateTextureMipmap(texture);
	}

	void getFramebufferAttachmentParameterIv(Enum target, Enum attachment, Enum pname, Int *params)
	{
		glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
	}

	void getNamedFramebufferAttachmentParameterIv(UInt p_framebuffer, Enum attachment, Enum pname, Int *params)
	{
		glGetNamedFramebufferAttachmentParameteriv(p_framebuffer, attachment, pname, params);
	}

	void getFramebufferParameterIv(Enum target, Enum pname, Int *params)
	{
		glGetFramebufferParameteriv(target, pname, params);
	}

	void getNamedFramebufferParameterIv(UInt p_framebuffer, Enum pname, Int *params)
	{
		glGetNamedFramebufferParameteriv(p_framebuffer, pname, params);
	}

	void getRenderbufferParameterIv(Enum target, Enum pname, Int *params)
	{
		glGetRenderbufferParameteriv(target, pname, params);
	}

	void getNamedRenderbufferParameterIv(UInt p_renderbuffer, Enum pname, Int *params)
	{
		glGetNamedRenderbufferParameteriv(p_renderbuffer, pname, params);
	}

	void invalidateFramebuffer(Enum target, SizeI num_attachments, const Enum *attachments)
	{
		glInvalidateFramebuffer(target, num_attachments, attachments);
	}

	void invalidateNamedFramebufferData(UInt p_framebuffer, SizeI num_attachments, const Enum *attachments)
	{
		glInvalidateNamedFramebufferData(p_framebuffer, num_attachments, attachments);
	}

	void invalidateSubFramebuffer(Enum target, SizeI num_attachments, const Enum *attachments, Int x, Int y, Int width, Int height)
	{
		glInvalidateSubFramebuffer(target, num_attachments, attachments, x, y, width, height);
	}

	void invalidateNamedFramebufferSubData(UInt p_framebuffer, SizeI num_attachments, const Enum *attachments, Int x, Int y, Int width, Int height)
	{
		glInvalidateNamedFramebufferSubData(p_framebuffer, num_attachments, attachments, x, y, width, height);
	}

	bool isFramebuffer(UInt p_framebuffer)
	{
		return glIsFramebuffer(p_framebuffer);
	}

	bool isRenderbuffer(UInt p_renderbuffer)
	{
		return glIsRenderbuffer(p_renderbuffer);
	}

	void renderbufferStorage(Enum target, Enum internalformat, SizeI width, SizeI height)
	{
		glRenderbufferStorage(target, internalformat, width, height);
	}

	void namedRenderbufferStorage(UInt p_renderbuffer, Enum internalformat, SizeI width, SizeI height)
	{
		glNamedRenderbufferStorage(p_renderbuffer, internalformat, width, height);
	}

	void renderbufferStorageMultisample(Enum target, SizeI samples, Enum internalformat, SizeI width, SizeI height)
	{
		glRenderbufferStorageMultisample(target, samples, internalformat, width, height);
	}

	void namedRenderbufferStorageMultisample(UInt p_renderbuffer, SizeI samples, Enum internalformat, SizeI width, SizeI height)
	{
		glNamedRenderbufferStorageMultisample(p_renderbuffer, samples, internalformat, width, height);
	}

	void sampleMaskI(UInt index, Bitfield mask)
	{
		glSampleMaski(index, mask);
	}

	#pragma endregion

	#pragma region Buffer Objects

	void bindBuffer(BufferType p_target, UInt p_buffer)
	{
		glBindBuffer(static_cast<Enum>(p_target), p_buffer);
	}

	void bindBufferBase(BufferType p_target, UInt p_index, UInt p_buffer)
	{
		glBindBufferBase(static_cast<Enum>(p_target), p_index, p_buffer);
	}

	void bindBufferRange(BufferType p_target, UInt p_index, UInt p_buffer, IntPtr p_offset, SizeIPtr p_size)
	{
		glBindBufferRange(static_cast<Enum>(p_target), p_index, p_buffer, p_offset, p_size);
	}

	void bindBuffersBase(BufferType p_target, UInt first, SizeI count, const UInt *buffers)
	{
		glBindBuffersBase(static_cast<Enum>(p_target), first, count, buffers);
	}

	void bindBuffersRange(BufferType p_target, UInt first, SizeI count, const UInt *buffers, IntPtr *offsets, SizeIPtr *sizes)
	{
		glBindBuffersRange(static_cast<Enum>(p_target), first, count, buffers, offsets, sizes);
	}

	void bindVertexBuffer(UInt binding_index, UInt buffer, IntPtr offset, SizeI stride)
	{
		glBindVertexBuffer(binding_index, buffer, offset, stride);
	}

	void vertexArrayVertexBuffer(UInt vao, UInt binding_index, UInt buffer, IntPtr offset, SizeI stride)
	{
		glVertexArrayVertexBuffer(vao, binding_index, buffer, offset, stride);
	}

	void bindVertexBuffers(UInt first, SizeI count, const UInt *buffers, const IntPtr *offsets, const SizeI *strides)
	{
		glBindVertexBuffers(first, count, buffers, offsets, strides);
	}

	void vertexArrayVertexBuffers(UInt vao, UInt first, SizeI count, const UInt *buffers, const IntPtr *offsets, const SizeI *strides)
	{
		glVertexArrayVertexBuffers(vao, first, count, buffers, offsets, strides);
	}

	void bufferData(BufferType p_target, SizeIPtr size, const Void *data, BufferUsage p_usage)
	{
		glBufferData(static_cast<Enum>(p_target), size, data, static_cast<Enum>(p_usage));
	}

	void namedBufferData(UInt target, SizeIPtr size, const Void *data, BufferUsage p_usage)
	{
		glNamedBufferData(target, size, data, static_cast<Enum>(p_usage));
	}

	void bufferStorage(BufferType p_target, SizeIPtr size, const Void *data, Bitfield flags)
	{
		glBufferStorage(static_cast<Enum>(p_target), size, data, flags);
	}

	void namedBufferStorage(UInt target, SizeIPtr size, const Void *data, Bitfield flags)
	{
		glBufferStorage(target, size, data, flags);
	}

	void bufferSubData(BufferType p_target, IntPtr offset, SizeIPtr size, const Void *data)
	{
		glBufferSubData(static_cast<Enum>(p_target), offset, size, data);
	}

	void namedBufferSubData(UInt target, IntPtr offset, SizeIPtr size, const Void *data)
	{
		glNamedBufferSubData(target, offset, size, data);
	}

	void clearBufferData(BufferType p_target, Enum internal_format, Enum format, Enum type, const Void *data)
	{
		glClearBufferData(static_cast<Enum>(p_target), internal_format, format, type, data);
	}

	void clearNamedBufferData(UInt target, Enum internal_format, Enum format, Enum type, const Void *data)
	{
		glClearNamedBufferData(target, internal_format, format, type, data);
	}

	void clearBufferSubData(BufferType p_target, Enum internal_format, IntPtr offset, SizeIPtr size, Enum format, Enum type, const Void *data)
	{
		glClearBufferSubData(static_cast<Enum>(p_target), internal_format, offset, size, format, type, data);
	}

	void clearNamedBufferSubData(UInt target, Enum internal_format, IntPtr offset, SizeIPtr size, Enum format, Enum type, const Void *data)
	{
		glClearNamedBufferSubData(target, internal_format, offset, size, format, type, data);
	}

	void copyBufferSubData(BufferType p_read_target, BufferType p_write_target, IntPtr readOffset, IntPtr writeOffset, SizeIPtr size)
	{
		glCopyBufferSubData(static_cast<Enum>(p_read_target), static_cast<Enum>(p_write_target), readOffset, writeOffset, size);
	}

	void copyNamedBufferSubData(UInt readBuffer, UInt writeBuffer, IntPtr readOffset, IntPtr writeOffset, SizeI size)
	{
		glCopyNamedBufferSubData(readBuffer, writeBuffer, readOffset, writeOffset, size);
	}

	void createBuffers(SizeI count, UInt *buffers)
	{
		glCreateBuffers(count, buffers);
	}

	void createVertexArrays(SizeI count, UInt *arrays)
	{
		glCreateVertexArrays(count, arrays);
	}

	void deleteBuffers(SizeI count, const UInt *buffers)
	{
		glDeleteBuffers(count, buffers);
	}

	void disableVertexAttribArray(UInt index)
	{
		glDisableVertexAttribArray(index);
	}

	void disableVertexArrayAttrib(UInt vao, UInt index)
	{
		glDisableVertexArrayAttrib(vao, index);
	}

	void drawArrays(Enum p_mode, Int p_first, SizeI p_count)
	{
		glDrawArrays(static_cast<Enum>(p_mode), p_first, p_count);
	}

	void drawArraysIndirect(DrawMode p_mode, const Void *p_indirect)
	{
		glDrawArraysIndirect(static_cast<Enum>(p_mode), p_indirect);
	}

	void drawArraysInstanced(DrawMode p_mode, Int p_first, SizeI p_count, SizeI p_primitive_count)
	{
		glDrawArraysInstanced(static_cast<Enum>(p_mode), p_first, p_count, p_primitive_count);
	}

	void drawArraysInstancedBaseInstance(DrawMode p_mode, Int p_first, SizeI p_count, SizeI p_primitive_count, UInt p_base_instance)
	{
		glDrawArraysInstancedBaseInstance(static_cast<Enum>(p_mode), p_first, p_count, p_primitive_count, p_base_instance);
	}

	void drawElements(DrawMode p_mode, SizeI p_count, DataType p_type, const Void *p_indices)
	{
		glDrawElements(static_cast<Enum>(p_mode), p_count, static_cast<Enum>(p_type), p_indices);
	}

	void drawElementsBaseVertex(DrawMode p_mode, SizeI p_count, DataType p_type, Void *p_indices, Int p_base_vertex)
	{
		glDrawElementsBaseVertex(static_cast<Enum>(p_mode), p_count, static_cast<Enum>(p_type), p_indices, p_base_vertex);
	}

	void drawElementsIndirect(DrawMode p_mode, DataType p_type, const void *p_indirect)
	{
		glDrawElementsIndirect(static_cast<Enum>(p_mode), static_cast<Enum>(p_type), p_indirect);
	}

	void drawElementsInstanced(DrawMode p_mode, SizeI p_count, DataType p_type, const void *p_indices, SizeI p_primitive_count)
	{
		glDrawElementsInstanced(static_cast<Enum>(p_mode), p_count, static_cast<Enum>(p_type), p_indices, p_primitive_count);
	}

	void drawElementsInstancedBaseInstance(DrawMode p_mode, SizeI p_count, DataType p_type, const void *p_indices, SizeI p_primitive_count, UInt p_base_instance)
	{
		glDrawElementsInstancedBaseInstance(static_cast<Enum>(p_mode), p_count, static_cast<Enum>(p_type), p_indices, p_primitive_count, p_base_instance);
	}

	void drawElementsInstancedBaseVertex(DrawMode p_mode, SizeI p_count, DataType p_type, Void *indices, SizeI p_primitive_count, Int p_base_vertex)
	{
		glDrawElementsInstancedBaseVertex(static_cast<Enum>(p_mode), p_count, static_cast<Enum>(p_type), indices, p_primitive_count, p_base_vertex);
	}

	void drawElementsInstancedBaseVertexBaseInstance(DrawMode p_mode, SizeI p_count, DataType p_type, Void *p_indices, SizeI p_primitive_count, Int p_base_vertex,
													 UInt     p_base_instance)
	{
		glDrawElementsInstancedBaseVertexBaseInstance(static_cast<Enum>(p_mode), p_count, static_cast<Enum>(p_type), p_indices, p_primitive_count, p_base_vertex,
													  p_base_instance);
	}

	void drawRangeElements(DrawMode p_mode, UInt p_start, UInt p_end, SizeI p_count, DataType p_type, const Void *indices)
	{
		glDrawRangeElements(static_cast<Enum>(p_mode), p_start, p_end, p_count, static_cast<Enum>(p_type), indices);
	}

	void drawRangeElementsBaseVertex(DrawMode p_mode, UInt p_start, UInt p_end, SizeI p_count, DataType p_type, Void *p_indices, Int p_base_vertex)
	{
		glDrawRangeElementsBaseVertex(static_cast<Enum>(p_mode), p_start, p_end, p_count, static_cast<Enum>(p_type), p_indices, p_base_vertex);
	}

	void enableVertexAttribArray(UInt index)
	{
		glEnableVertexAttribArray(index);
	}

	void enableVertexArrayAttrib(UInt vao, UInt index)
	{
		glEnableVertexArrayAttrib(vao, index);
	}

	void flushMappedBufferRange(Enum target, IntPtr offset, SizeIPtr length)
	{
		glFlushMappedBufferRange(target, offset, length);
	}

	void flushMappedNamedBufferRange(UInt buffer, IntPtr offset, SizeIPtr length)
	{
		glFlushMappedNamedBufferRange(buffer, offset, length);
	}

	void genBuffers(SizeI n, UInt *buffers)
	{
		glGenBuffers(n, buffers);
	}

	void getBufferParameterIv(Enum target, Enum value, Int *data)
	{
		glGetBufferParameteriv(target, value, data);
	}

	void getBufferParameterI64V(Enum target, Enum value, Int64 *data)
	{
		glGetBufferParameteri64v(target, value, data);
	}

	void getNamedBufferParameterIv(UInt buffer, Enum pname, Int *params)
	{
		glGetNamedBufferParameteriv(buffer, pname, params);
	}

	void getNamedBufferParameterI64V(UInt buffer, Enum pname, Int64 *params)
	{
		glGetNamedBufferParameteri64v(buffer, pname, params);
	}

	void getBufferPointerV(Enum target, Enum pname, Void **params)
	{
		glGetBufferPointerv(target, pname, params);
	}

	void getNamedBufferPointerV(UInt buffer, Enum pname, void **params)
	{
		glGetNamedBufferPointerv(buffer, pname, params);
	}

	void getBufferSubData(Enum target, IntPtr offset, SizeIPtr size, Void *data)
	{
		glGetBufferSubData(target, offset, size, data);
	}

	void getNamedBufferSubData(UInt buffer, IntPtr offset, SizeI size, void *data)
	{
		glGetNamedBufferSubData(buffer, offset, size, data);
	}

	void getVertexArrayIndexed64Iv(UInt vao, UInt index, Enum pname, Int64 *param)
	{
		glGetVertexArrayIndexed64iv(vao, index, pname, param);
	}

	void getVertexArrayIndexedIv(UInt vao, UInt index, Enum pname, Int *param)
	{
		glGetVertexArrayIndexediv(vao, index, pname, param);
	}

	void getVertexArrayIv(UInt vao, Enum pname, Int *param)
	{
		glGetVertexArrayiv(vao, pname, param);
	}

	void getVertexAttribDv(UInt index, Enum pname, Double *params)
	{
		glGetVertexAttribdv(index, pname, params);
	}

	void getVertexAttribFv(UInt index, Enum pname, Float *params)
	{
		glGetVertexAttribfv(index, pname, params);
	}

	void getVertexAttribIv(UInt index, Enum pname, Int *params)
	{
		glGetVertexAttribiv(index, pname, params);
	}

	void getVertexAttribIiv(UInt index, Enum pname, Int *params)
	{
		glGetVertexAttribIiv(index, pname, params);
	}

	void getVertexAttribIuiv(UInt index, Enum pname, UInt *params)
	{
		glGetVertexAttribIuiv(index, pname, params);
	}

	void getVertexAttribLdv(UInt index, Enum pname, Double *params)
	{
		glGetVertexAttribLdv(index, pname, params);
	}

	void getVertexAttribPointerV(UInt index, Enum pname, Void **pointer)
	{
		glGetVertexAttribPointerv(index, pname, pointer);
	}

	void invalidateBufferData(UInt buffer)
	{
		glInvalidateBufferData(buffer);
	}

	void invalidateBufferSubData(UInt buffer, IntPtr offset, SizeIPtr length)
	{
		glInvalidateBufferSubData(buffer, offset, length);
	}

	Bool isBuffer(UInt buffer)
	{
		return glIsBuffer(buffer);
	}

	Void *mapBuffer(Enum target, Enum access)
	{
		return glMapBuffer(target, access);
	}

	Void *mapNamedBuffer(UInt buffer, Enum access)
	{
		return glMapNamedBuffer(buffer, access);
	}

	Void *mapBufferRange(Enum target, IntPtr offset, SizeIPtr length, Bitfield access)
	{
		return glMapBufferRange(target, access, offset, length);
	}

	Void *mapNamedBufferRange(UInt buffer, IntPtr offset, SizeIPtr length, Bitfield access)
	{
		return glMapNamedBufferRange(buffer, access, offset, length);
	}

	void multiDrawArrays(Enum mode, const Int *first, const SizeI *count, SizeI draw_count)
	{
		glMultiDrawArrays(mode, first, count, draw_count);
	}

	void multiDrawArraysIndirect(Enum mode, const void *indirect, SizeI draw_count, SizeI stride)
	{
		glMultiDrawArraysIndirect(mode, indirect, draw_count, stride);
	}

	void multiDrawElements(Enum mode, const SizeI *count, Enum type, const Void *const *indices, SizeI draw_count)
	{
		glMultiDrawElements(mode, count, type, indices, draw_count);
	}

	void multiDrawElementsBaseVertex(Enum mode, const SizeI *count, Enum type, const Void *const *indices, SizeI draw_count, const Int *base_vertex)
	{
		glMultiDrawElementsBaseVertex(mode, count, type, indices, draw_count, base_vertex);
	}

	void multiDrawElementsIndirect(Enum mode, Enum type, const void *indirect, SizeI draw_count, SizeI stride)
	{
		glMultiDrawElementsIndirect(mode, type, indirect, draw_count, stride);
	}

	void patchParameterI(Enum pname, Int value)
	{
		glPatchParameteri(pname, value);
	}

	void patchParameterFv(Enum pname, const Float *p_values)
	{
		glPatchParameterfv(pname, p_values);
	}

	void primitiveRestartIndex(UInt index)
	{
		glPrimitiveRestartIndex(index);
	}

	void provokingVertex(Enum provokeMode)
	{
		glProvokingVertex(provokeMode);
	}

	Bool unmapBuffer(Enum target)
	{
		return glUnmapBuffer(target);
	}

	Bool unmapNamedBuffer(UInt buffer)
	{
		return glUnmapNamedBuffer(buffer);
	}

	void vertexArrayElementBuffer(UInt vao, UInt buffer)
	{
		glVertexArrayElementBuffer(vao, buffer);
	}

	void vertexAttrib1f(UInt index, Float v0)
	{
		glVertexAttrib1f(index, v0);
	}

	void vertexAttrib1s(UInt index, Short v0)
	{
		glVertexAttrib1s(index, v0);
	}

	void vertexAttrib1d(UInt index, Double v0)
	{
		glVertexAttrib1d(index, v0);
	}

	void vertexAttribI1i(UInt index, Int v0)
	{
		glVertexAttribI1i(index, v0);
	}

	void vertexAttribI1ui(UInt index, UInt v0)
	{
		glVertexAttribI1ui(index, v0);
	}

	void vertexAttrib2f(UInt index, Float v0, Float v1)
	{
		glVertexAttrib2f(index, v0, v1);
	}

	void vertexAttrib2s(UInt index, Short v0, Short v1)
	{
		glVertexAttrib2s(index, v0, v1);
	}

	void vertexAttrib2d(UInt index, Double v0, Double v1)
	{
		glVertexAttrib2d(index, v0, v1);
	}

	void vertexAttribI2i(UInt index, Int v0, Int v1)
	{
		glVertexAttribI2i(index, v0, v1);
	}

	void vertexAttribI2ui(UInt index, UInt v0, UInt v1)
	{
		glVertexAttribI2ui(index, v0, v1);
	}

	void vertexAttrib3f(UInt index, Float v0, Float v1, Float v2)
	{
		glVertexAttrib3f(index, v0, v1, v2);
	}

	void vertexAttrib3s(UInt index, Short v0, Short v1, Short v2)
	{
		glVertexAttrib3s(index, v0, v1, v2);
	}

	void vertexAttrib3d(UInt index, Double v0, Double v1, Double v2)
	{
		glVertexAttrib3d(index, v0, v1, v2);
	}

	void vertexAttribI3i(UInt index, Int v0, Int v1, Int v2)
	{
		glVertexAttribI3i(index, v0, v1, v2);
	}

	void vertexAttribI3ui(UInt index, UInt v0, UInt v1, UInt v2)
	{
		glVertexAttribI3ui(index, v0, v1, v2);
	}

	void vertexAttrib4f(UInt index, Float v0, Float v1, Float v2, Float v3)
	{
		glVertexAttrib4f(index, v0, v1, v2, v3);
	}

	void vertexAttrib4s(UInt index, Short v0, Short v1, Short v2, Short v3)
	{
		glVertexAttrib4s(index, v0, v1, v2, v3);
	}

	void vertexAttrib4d(UInt index, Double v0, Double v1, Double v2, Double v3)
	{
		glVertexAttrib4d(index, v0, v1, v2, v3);
	}

	void vertexAttrib4Nub(UInt index, UByte v0, UByte v1, UByte v2, UByte v3)
	{
		glVertexAttrib4Nub(index, v0, v1, v2, v3);
	}

	void vertexAttribI4i(UInt index, Int v0, Int v1, Int v2, Int v3)
	{
		glVertexAttribI4i(index, v0, v1, v2, v3);
	}

	void vertexAttribI4ui(UInt index, UInt v0, UInt v1, UInt v2, UInt v3)
	{
		glVertexAttribI4ui(index, v0, v1, v2, v3);
	}

	void vertexAttribL1d(UInt index, Double v0)
	{
		glVertexAttribL1d(index, v0);
	}

	void vertexAttribL2d(UInt index, Double v0, Double v1)
	{
		glVertexAttribL2d(index, v0, v1);
	}

	void vertexAttribL3d(UInt index, Double v0, Double v1, Double v2)
	{
		glVertexAttribL3d(index, v0, v1, v2);
	}

	void vertexAttribL4d(UInt index, Double v0, Double v1, Double v2, Double v3)
	{
		glVertexAttribL4d(index, v0, v1, v2, v3);
	}

	void vertexAttrib1fv(UInt index, const Float *v)
	{
		glVertexAttrib1fv(index, v);
	}

	void vertexAttrib1sv(UInt index, const Short *v)
	{
		glVertexAttrib1sv(index, v);
	}

	void vertexAttrib1dv(UInt index, const Double *v)
	{
		glVertexAttrib1dv(index, v);
	}

	void vertexAttribI1iv(UInt index, const Int *v)
	{
		glVertexAttribI1iv(index, v);
	}

	void vertexAttribI1uiv(UInt index, const UInt *v)
	{
		glVertexAttribI1uiv(index, v);
	}

	void vertexAttrib2fv(UInt index, const Float *v)
	{
		glVertexAttrib2fv(index, v);
	}

	void vertexAttrib2sv(UInt index, const Short *v)
	{
		glVertexAttrib2sv(index, v);
	}

	void vertexAttrib2dv(UInt index, const Double *v)
	{
		glVertexAttrib2dv(index, v);
	}

	void vertexAttribI2iv(UInt index, const Int *v)
	{
		glVertexAttribI2iv(index, v);
	}

	void vertexAttribI2uiv(UInt index, const UInt *v)
	{
		glVertexAttribI2uiv(index, v);
	}

	void vertexAttrib3fv(UInt index, const Float *v)
	{
		glVertexAttrib3fv(index, v);
	}

	void vertexAttrib3sv(UInt index, const Short *v)
	{
		glVertexAttrib3sv(index, v);
	}

	void vertexAttrib3dv(UInt index, const Double *v)
	{
		glVertexAttrib3dv(index, v);
	}

	void vertexAttribI3iv(UInt index, const Int *v)
	{
		glVertexAttribI3iv(index, v);
	}

	void vertexAttribI3uiv(UInt index, const UInt *v)
	{
		glVertexAttribI3uiv(index, v);
	}

	void vertexAttrib4fv(UInt index, const Float *v)
	{
		glVertexAttrib4fv(index, v);
	}

	void vertexAttrib4sv(UInt index, const Short *v)
	{
		glVertexAttrib4sv(index, v);
	}

	void vertexAttrib4dv(UInt index, const Double *v)
	{
		glVertexAttrib4dv(index, v);
	}

	void vertexAttrib4iv(UInt index, const Int *v)
	{
		glVertexAttrib4iv(index, v);
	}

	void vertexAttrib4bv(UInt index, const Byte *v)
	{
		glVertexAttrib4bv(index, v);
	}

	void vertexAttrib4ubv(UInt index, const UByte *v)
	{
		glVertexAttrib4ubv(index, v);
	}

	void vertexAttrib4usv(UInt index, const UShort *v)
	{
		glVertexAttrib4usv(index, v);
	}

	void vertexAttrib4uiv(UInt index, const UInt *v)
	{
		glVertexAttrib4uiv(index, v);
	}

	void vertexAttrib4Nbv(UInt index, const Byte *v)
	{
		glVertexAttrib4Nbv(index, v);
	}

	void vertexAttrib4Nsv(UInt index, const Short *v)
	{
		glVertexAttrib4Nsv(index, v);
	}

	void vertexAttrib4Niv(UInt index, const Int *v)
	{
		glVertexAttrib4Niv(index, v);
	}

	void vertexAttrib4Nubv(UInt index, const UByte *v)
	{
		glVertexAttrib4Nubv(index, v);
	}

	void vertexAttrib4Nusv(UInt index, const UShort *v)
	{
		glVertexAttrib4Nusv(index, v);
	}

	void vertexAttrib4Nuiv(UInt index, const UInt *v)
	{
		glVertexAttrib4Nuiv(index, v);
	}

	void vertexAttribI4bv(UInt index, const Byte *v)
	{
		glVertexAttribI4bv(index, v);
	}

	void vertexAttribI4ubv(UInt index, const UByte *v)
	{
		glVertexAttribI4ubv(index, v);
	}

	void vertexAttribI4sv(UInt index, const Short *v)
	{
		glVertexAttribI4sv(index, v);
	}

	void vertexAttribI4usv(UInt index, const UShort *v)
	{
		glVertexAttribI4usv(index, v);
	}

	void vertexAttribI4iv(UInt index, const Int *v)
	{
		glVertexAttribI4iv(index, v);
	}

	void vertexAttribI4uiv(UInt index, const UInt *v)
	{
		glVertexAttribI4uiv(index, v);
	}

	void vertexAttribL1dv(UInt index, const Double *v)
	{
		glVertexAttrib1dv(index, v);
	}

	void vertexAttribL2dv(UInt index, const Double *v)
	{
		glVertexAttrib2dv(index, v);
	}

	void vertexAttribL3dv(UInt index, const Double *v)
	{
		glVertexAttrib3dv(index, v);
	}

	void vertexAttribL4dv(UInt index, const Double *v)
	{
		glVertexAttrib4dv(index, v);
	}

	void vertexAttribP1ui(UInt index, Enum type, Bool normalized, UInt value)
	{
		glVertexAttribP1ui(index, type, normalized, value);
	}

	void vertexAttribP2ui(UInt index, Enum type, Bool normalized, UInt value)
	{
		glVertexAttribP2ui(index, type, normalized, value);
	}

	void vertexAttribP3ui(UInt index, Enum type, Bool normalized, UInt value)
	{
		glVertexAttribP3ui(index, type, normalized, value);
	}

	void vertexAttribP4ui(UInt index, Enum type, Bool normalized, UInt value)
	{
		glVertexAttribP4ui(index, type, normalized, value);
	}

	void vertexAttribBinding(UInt attrib_index, UInt binding_index)
	{
		glVertexAttribBinding(attrib_index, binding_index);
	}

	void vertexArrayAttribBinding(UInt vao, UInt attrib_index, UInt binding_index)
	{
		glVertexArrayAttribBinding(vao, attrib_index, binding_index);
	}

	void vertexAttribDivisor(UInt index, UInt divisor)
	{
		glVertexAttribDivisor(index, divisor);
	}

	void vertexAttribFormat(UInt attrib_index, Int size, DataType p_type, Bool normalized, UInt relative_offset)
	{
		glVertexAttribFormat(attrib_index, size, static_cast<Enum>(p_type), normalized, relative_offset);
	}

	void vertexAttribIFormat(UInt attrib_index, Int size, DataType p_type, UInt relative_offset)
	{
		glVertexAttribIFormat(attrib_index, size, static_cast<Enum>(p_type), relative_offset);
	}

	void vertexAttribLFormat(UInt attrib_index, Int size, DataType p_type, UInt relative_offset)
	{
		glVertexAttribLFormat(attrib_index, size, static_cast<Enum>(p_type), relative_offset);
	}

	void vertexArrayAttribFormat(UInt vao, UInt attrib_index, Int size, DataType p_type, Bool normalized, UInt relative_offset)
	{
		glVertexArrayAttribFormat(vao, attrib_index, size, static_cast<Enum>(p_type), normalized, relative_offset);
	}

	void vertexArrayAttribIFormat(UInt vao, UInt attrib_index, Int size, DataType p_type, UInt relative_offset)
	{
		glVertexArrayAttribIFormat(vao, attrib_index, size, static_cast<Enum>(p_type), relative_offset);
	}

	void vertexArrayAttribLFormat(UInt vao, UInt attrib_index, Int size, DataType p_type, UInt relative_offset)
	{
		glVertexArrayAttribLFormat(vao, attrib_index, size, static_cast<Enum>(p_type), relative_offset);
	}

	void vertexAttribPointer(UInt index, Int size, DataType p_type, Bool normalized, SizeI stride, const Void *pointer)
	{
		glVertexAttribPointer(index, size, static_cast<Enum>(p_type), normalized, stride, pointer);
	}

	void vertexAttribIPointer(UInt index, Int size, DataType p_type, SizeI stride, const Void *pointer)
	{
		glVertexAttribIPointer(index, size, static_cast<Enum>(p_type), stride, pointer);
	}

	void vertexAttribLPointer(UInt index, Int size, DataType p_type, SizeI stride, const Void *pointer)
	{
		glVertexAttribLPointer(index, size, static_cast<Enum>(p_type), stride, pointer);
	}

	void vertexBindingDivisor(UInt binding_index, UInt divisor)
	{
		glVertexAttribBinding(binding_index, divisor);
	}

	void vertexArrayBindingDivisor(UInt vao, UInt binding_index, UInt divisor)
	{
		glVertexArrayBindingDivisor(vao, binding_index, divisor);
	}

	#pragma endregion

	#pragma region Vertex Array Objects

	void bindVertexArray(UInt array)
	{
		glBindVertexArray(array);
	}

	void deleteVertexArrays(SizeI n, const UInt *arrays)
	{
		glDeleteVertexArrays(n, arrays);
	}

	void genVertexArrays(SizeI n, UInt *arrays)
	{
		glGenVertexArrays(n, arrays);
	}

	Bool isVertexArray(UInt array)
	{
		return glIsVertexArray(array);
	}

	#pragma endregion

	#pragma region Textures

	void activeTexture(TextureUnit p_texture_unit)
	{
		glActiveTexture(static_cast<Enum>(p_texture_unit));
	}

	void bindImageTexture(UInt unit, UInt texture, Int level, Bool layered, Int layer, Enum access, Enum format)
	{
		glBindImageTexture(unit, texture, level, layered, layer, access, format);
	}

	void bindImageTextures(UInt first, SizeI count, const UInt *textures)
	{
		glBindImageTextures(first, count, textures);
	}

	void bindTexture(TextureType p_target, UInt texture)
	{
		glBindTexture(static_cast<Enum>(p_target), texture);
	}

	void bindTextureUnit(UInt p_texture_unit, UInt texture)
	{
		glBindTextureUnit(p_texture_unit, texture);
	}

	void bindTextures(UInt first, SizeI count, const UInt *textures)
	{
		glBindTextures(first, count, textures);
	}

	void clearTexImage(UInt texture, Int level, Enum format, Enum type, const void *data)
	{
		glClearTexImage(texture, level, format, type, data);
	}

	void clearTexSubImage(UInt        texture, Int level, Int x_offset, Int y_offset, Int z_offset, SizeI width, SizeI height, SizeI depth, Enum format, Enum type,
						  const void *data)
	{
		glClearTexSubImage(texture, level, x_offset, y_offset, z_offset, width, height, depth, format, type, data);
	}

	void compressedTexImage1D(Enum target, Int level, Enum internalformat, SizeI width, Int border, SizeI imageSize, const Void *data)
	{
		glCompressedTexImage1D(target, level, internalformat, width, border, imageSize, data);
	}

	void compressedTexImage2D(Enum target, Int level, Enum internalformat, SizeI width, SizeI height, Int border, SizeI imageSize, const Void *data)
	{
		glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
	}

	void compressedTexImage3D(Enum target, Int level, Enum internalformat, SizeI width, SizeI height, SizeI depth, Int border, SizeI imageSize, const Void *data)
	{
		glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
	}

	void compressedTexSubImage1D(Enum target, Int level, Int x_offset, SizeI width, Enum format, SizeI imageSize, const Void *data)
	{
		glCompressedTexSubImage1D(target, level, x_offset, width, format, imageSize, data);
	}

	void compressedTextureSubImage1D(UInt texture, Int level, Int x_offset, SizeI width, Enum format, SizeI imageSize, const void *data)
	{
		glCompressedTextureSubImage1D(texture, level, x_offset, width, format, imageSize, data);
	}

	void compressedTexSubImage2D(Enum target, Int level, Int x_offset, Int y_offset, SizeI width, SizeI height, Enum format, SizeI imageSize, const Void *data)
	{
		glCompressedTexSubImage2D(target, level, x_offset, y_offset, width, height, format, imageSize, data);
	}

	void compressedTextureSubImage2D(UInt texture, Int level, Int x_offset, Int y_offset, SizeI width, SizeI height, Enum format, SizeI imageSize, const void *data)
	{
		glCompressedTexSubImage2D(texture, level, x_offset, y_offset, width, height, format, imageSize, data);
	}

	void compressedTexSubImage3D(Enum target, Int level, Int x_offset, Int y_offset, Int z_offset, SizeI width, SizeI height, SizeI depth, Enum format, SizeI imageSize,
								 const Void *data)
	{
		glCompressedTexSubImage3D(target, level, x_offset, y_offset, z_offset, width, height, depth, format, imageSize, data);
	}

	void compressedTextureSubImage3D(UInt  texture, Int           level, Int x_offset, Int y_offset, Int z_offset, SizeI width, SizeI height, SizeI depth, Enum format,
									 SizeI imageSize, const void *data)
	{
		glCompressedTextureSubImage3D(texture, level, x_offset, y_offset, z_offset, width, height, depth, format, imageSize, data);
	}

	void copyImageSubData(UInt srcName, Enum srcTarget, Int  srcLevel, Int    srcX, Int srcY, Int srcZ, UInt dstName, Enum dstTarget, Int dstLevel, Int dstX, Int dstY,
						  Int  dstZ, SizeI   srcWidth, SizeI srcHeight, SizeI srcDepth)
	{
		glCopyImageSubData(srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth);
	}

	void copyTexImage1D(Enum target, Int level, Enum internalformat, Int x, Int y, SizeI width, Int border)
	{
		glCopyTexImage1D(target, level, internalformat, x, y, width, border);
	}

	void copyTexImage2D(Enum target, Int level, Enum internalformat, Int x, Int y, SizeI width, SizeI height, Int border)
	{
		glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
	}

	void copyTexSubImage1D(Enum target, Int level, Int x_offset, Int x, Int y, SizeI width)
	{
		glCopyTexSubImage1D(target, level, x_offset, x, y, width);
	}

	void copyTextureSubImage1D(UInt texture, Int level, Int x_offset, Int x, Int y, SizeI width)
	{
		glCopyTextureSubImage1D(texture, level, x_offset, x, y, width);
	}

	void copyTexSubImage2D(Enum target, Int level, Int x_offset, Int y_offset, Int x, Int y, SizeI width, SizeI height)
	{
		glCopyTexImage2D(target, level, x_offset, y_offset, x, y, width, height);
	}

	void copyTextureSubImage2D(UInt texture, Int level, Int x_offset, Int y_offset, Int x, Int y, SizeI width, SizeI height)
	{
		glCopyTextureSubImage2D(texture, level, x_offset, y_offset, x, y, width, height);
	}

	void copyTexSubImage3D(Enum target, Int level, Int x_offset, Int y_offset, Int z_offset, Int x, Int y, SizeI width, SizeI height)
	{
		glCopyTexSubImage3D(target, level, x_offset, y_offset, z_offset, x, y, width, height);
	}

	void copyTextureSubImage3D(UInt texture, Int level, Int x_offset, Int y_offset, Int z_offset, Int x, Int y, SizeI width, SizeI height)
	{
		glCopyTextureSubImage3D(texture, level, x_offset, y_offset, z_offset, x, y, width, height);
	}

	void createTextures(TextureType p_target, SizeI n, UInt *textures)
	{
		glCreateTextures(static_cast<Enum>(p_target), n, textures);
	}

	void deleteTextures(SizeI n, const UInt *textures)
	{
		glDeleteTextures(n, textures);
	}

	void genTextures(SizeI n, UInt *textures)
	{
		glGenTextures(n, textures);
	}

	void getCompressedTexImage(Enum target, Int level, Void *pixels)
	{
		glGetCompressedTexImage(target, level, pixels);
	}

	void getnCompressedTexImage(Enum target, Int level, SizeI bufSize, void *pixels)
	{
		glGetnCompressedTexImage(target, level, bufSize, pixels);
	}

	void getCompressedTextureImage(UInt texture, Int level, SizeI bufSize, void *pixels)
	{
		glGetCompressedTextureImage(texture, level, bufSize, pixels);
	}

	void getCompressedTextureSubImage(UInt  texture, Int level, Int x_offset, Int y_offset, Int z_offset, SizeI width, SizeI height, SizeI depth, SizeI bufSize,
									  void *pixels)
	{
		glGetCompressedTextureSubImage(texture, level, x_offset, y_offset, z_offset, width, height, depth, bufSize, pixels);
	}

	void getTexImage(TextureType p_target, Int level, Format p_format, DataType p_type, Void *pixels)
	{
		glGetTexImage(static_cast<Enum>(p_target), level, static_cast<Enum>(p_format), static_cast<Enum>(p_type), pixels);
	}

	void getnTexImage(Enum target, Int level, Enum format, Enum type, SizeI bufSize, void *pixels)
	{
		glGetnTexImage(target, level, format, type, bufSize, pixels);
	}

	void getTextureImage(UInt texture, Int level, Format p_format, DataType p_type, SizeI bufSize, void *pixels)
	{
		glGetTextureImage(texture, level, static_cast<Enum>(p_format), static_cast<Enum>(p_type), bufSize, pixels);
	}

	void getTexLevelParameterFv(Enum target, Int level, Enum pname, Float *params)
	{
		glGetTexLevelParameterfv(target, level, pname, params);
	}

	void getTexLevelParameterIv(Enum target, Int level, Enum pname, Int *params)
	{
		glGetTexLevelParameteriv(target, level, pname, params);
	}

	void getTextureLevelParameterFv(UInt texture, Int level, Enum pname, Float *params)
	{
		glGetTextureLevelParameterfv(texture, level, pname, params);
	}

	void getTextureLevelParameterIv(UInt texture, Int level, Enum pname, Int *params)
	{
		glGetTextureLevelParameteriv(texture, level, pname, params);
	}

	void getTexParameterFv(Enum target, Enum pname, Float *params)
	{
		glGetTexParameterfv(target, pname, params);
	}

	void getTexParameterIv(Enum target, Enum pname, Int *params)
	{
		glGetTexParameteriv(target, pname, params);
	}

	void getTexParameterIiv(Enum target, Enum pname, Int *params)
	{
		glGetTexParameterIiv(target, pname, params);
	}

	void getTexParameterIuiv(Enum target, Enum pname, UInt *params)
	{
		glGetTexParameterIuiv(target, pname, params);
	}

	void getTextureParameterFv(UInt texture, Enum pname, Float *params)
	{
		glGetTextureParameterfv(texture, pname, params);
	}

	void getTextureParameterIv(UInt texture, Enum pname, Int *params)
	{
		glGetTextureParameteriv(texture, pname, params);
	}

	void getTextureParameterIiv(UInt texture, Enum pname, Int *params)
	{
		glGetTextureParameterIiv(texture, pname, params);
	}

	void getTextureParameterIuiv(UInt texture, Enum pname, UInt *params)
	{
		glGetTextureParameterIuiv(texture, pname, params);
	}

	void getTextureSubImage(UInt  texture, Int   level, Int x_offset, Int y_offset, Int z_offset, SizeI width, SizeI height, SizeI depth, Enum format, Enum type,
							SizeI bufSize, void *pixels)
	{
		glGetTextureSubImage(texture, level, x_offset, y_offset, z_offset, width, height, depth, format, type, bufSize, pixels);
	}

	void invalidateTexImage(UInt texture, Int level)
	{
		glInvalidateTexImage(texture, level);
	}

	void invalidateTexSubImage(UInt texture, Int level, Int x_offset, Int y_offset, Int z_offset, SizeI width, SizeI height, SizeI depth)
	{
		glInvalidateTexSubImage(texture, level, x_offset, y_offset, z_offset, width, height, depth);
	}

	Bool isTexture(UInt texture)
	{
		return glIsTexture(texture);
	}

	void texBuffer(TextureType p_target, Enum internalFormat, UInt buffer)
	{
	}

	void textureBuffer(UInt texture, Enum internalformat, UInt buffer)
	{
	}

	void texBufferRange(TextureType p_target, Enum internalFormat, UInt buffer, IntPtr offset, SizeIPtr size)
	{
	}

	void textureBufferRange(UInt texture, Enum internalformat, UInt buffer, IntPtr offset, SizeI size)
	{
	}

	void texImage1D(TextureType p_target, Int level, Int internalFormat, SizeI width, Int border, Enum format, Enum type, const Void *data)
	{
	}

	void texImage2D(TextureType p_target, Int level, Format p_internal_format, SizeI width, SizeI height, Int border, Format p_format, DataType p_type, const Void *data)
	{
		glTexImage2D(static_cast<Enum>(p_target), level, static_cast<Int>(p_internal_format), width, height, border, static_cast<Enum>(p_format),
					 static_cast<Enum>(p_type), data);
	}

	void texImage2DMultisample(TextureType p_target, SizeI samples, Enum internalformat, SizeI width, SizeI height, Bool fixed_sample_locations)
	{
	}

	void texImage3D(TextureType p_target, Int level, Int internalFormat, SizeI width, SizeI height, SizeI depth, Int border, Enum format, Enum type, const Void *data)
	{
		glTexImage3D(static_cast<Enum>(p_target), level, internalFormat, width, height, depth, border, format, type, data);
	}

	void texImage3DMultisample(TextureType p_target, SizeI samples, Enum internalformat, SizeI width, SizeI height, SizeI depth, Bool fixed_sample_locations)
	{
		glTexImage3DMultisample(static_cast<Enum>(p_target), samples, internalformat, width, height, depth, fixed_sample_locations);
	}

	void texParameterf(TextureType p_target, SamplerParameter p_sampler_parameter, Float param)
	{
		glTexParameterf(static_cast<Enum>(p_target), static_cast<Enum>(p_sampler_parameter), param);
	}

	void texParameteri(TextureType p_target, SamplerParameter p_sampler_parameter, Int param)
	{
		glTexParameteri(static_cast<Enum>(p_target), static_cast<Enum>(p_sampler_parameter), param);
	}

	void textureParameterf(UInt texture, SamplerParameter p_sampler_parameter, Float param)
	{
		glTextureParameterf(texture, static_cast<Enum>(p_sampler_parameter), param);
	}

	void textureParameteri(UInt texture, SamplerParameter p_sampler_parameter, Int param)
	{
		glTextureParameteri(texture, static_cast<Enum>(p_sampler_parameter), param);
	}

	void texParameterFv(TextureType p_target, SamplerParameter p_sampler_parameter, const Float *params)
	{
		glTextureParameterfv(static_cast<Enum>(p_target), static_cast<Enum>(p_sampler_parameter), params);
	}

	void texParameterIv(TextureType p_target, SamplerParameter p_sampler_parameter, const Int *params)
	{
	}

	void texParameterIiv(TextureType p_target, SamplerParameter p_sampler_parameter, const Int *params)
	{
	}

	void texParameterIuiv(TextureType p_target, SamplerParameter p_sampler_parameter, const UInt *params)
	{
	}

	void textureParameterFv(UInt texture, SamplerParameter p_sampler_parameter, const Float *param_texture)
	{
	}

	void textureParameterIv(UInt texture, SamplerParameter p_sampler_parameter, const Int *param)
	{
	}

	void textureParameterIiv(UInt texture, SamplerParameter p_sampler_parameter, const Int *params)
	{
	}

	void textureParameterIuiv(UInt texture, SamplerParameter p_sampler_parameter, const UInt *params)
	{
	}

	void texStorage1D(TextureType p_target, SizeI levels, Enum internalformat, SizeI width)
	{
	}

	void textureStorage1D(UInt texture, SizeI levels, Enum internalformat, SizeI width)
	{
	}

	void texStorage2D(TextureType p_target, SizeI levels, Format p_internal_format, SizeI width, SizeI height)
	{
		glTexStorage2D(static_cast<Enum>(p_target), levels, static_cast<Enum>(p_internal_format), width, height);
	}

	void textureStorage2D(UInt texture, SizeI levels, Format p_internal_format, SizeI width, SizeI height)
	{
		glTextureStorage2D(texture, levels, static_cast<Enum>(p_internal_format), width, height);
	}

	void texStorage2DMultisample(TextureType p_target, SizeI samples, Enum internalformat, SizeI width, SizeI height, Bool fixed_sample_locations)
	{
	}

	void textureStorage2DMultisample(UInt texture, SizeI samples, Enum internalformat, SizeI width, SizeI height, Bool fixed_sample_locations)
	{
	}

	void texStorage3D(TextureType p_target, SizeI levels, Enum internalformat, SizeI width, SizeI height, SizeI depth)
	{
	}

	void textureStorage3D(UInt texture, SizeI levels, Enum internalformat, SizeI width, SizeI height, SizeI depth)
	{
	}

	void texStorage3DMultisample(TextureType p_target, SizeI samples, Enum internalformat, SizeI width, SizeI height, SizeI depth, Bool fixed_sample_locations)
	{
	}

	void textureStorage3DMultisample(UInt texture, SizeI samples, Enum internalformat, SizeI width, SizeI height, SizeI depth, Bool fixed_sample_locations)
	{
	}

	void texSubImage1D(TextureType p_target, Int level, Int x_offset, SizeI width, Enum format, Enum type, const Void *pixels)
	{
	}

	void textureSubImage1D(UInt texture, Int level, Int x_offset, SizeI width, Enum format, Enum type, const void *pixels)
	{
	}

	void texSubImage2D(TextureType p_target, Int level, Int x_offset, Int y_offset, SizeI width, SizeI height, Enum format, Enum type, const Void *pixels)
	{
	}

	void textureSubImage2D(UInt texture, Int level, Int x_offset, Int y_offset, SizeI width, SizeI height, Format p_format, DataType p_type, const void *pixels)
	{
		glTextureSubImage2D(texture, level, x_offset, y_offset, width, height, static_cast<Enum>(p_format), static_cast<Enum>(p_type), pixels);
	}

	void texSubImage3D(TextureType p_target, Int       level, Int x_offset, Int y_offset, Int z_offset, SizeI width, SizeI height, SizeI depth, Format p_format,
					   DataType    p_type, const Void *pixels)
	{
	}

	void textureSubImage3D(UInt texture, Int level, Int x_offset, Int y_offset, Int z_offset, SizeI width, SizeI height, SizeI depth, Format p_format, DataType p_type,
						   const void *pixels)
	{
	}

	void textureView(UInt p_texture, TextureType p_target, UInt p_origin_texture, Format p_internal_format, UInt p_min_level, UInt p_num_levels, UInt p_min_layer, UInt num_layers)
	{
		glTextureView(p_texture, static_cast<Enum>(p_target), p_origin_texture, static_cast<Enum>(p_internal_format), p_min_level, p_num_levels, p_min_layer, num_layers);
	}
	#pragma endregion

	#pragma region State Management

	void blendColor(Float red, Float green, Float blue, Float alpha)
	{
	}

	void blendEquation(Enum mode)
	{
	}

	void blendEquationI(UInt buffer, Enum mode)
	{
	}

	void blendEquationSeparate(Enum mode_RGB, Enum mode_alpha)
	{
	}

	void blendEquationSeparateI(UInt buffer, Enum mode_RGB, Enum mode_alpha)
	{
	}

	void blendFunc(BlendFunc p_s_factor, BlendFunc p_d_factor)
	{
		glBlendFunc(static_cast<Enum>(p_s_factor), static_cast<Enum>(p_d_factor));
	}

	void blendFuncI(UInt buffer, Enum s_factor, Enum d_factor)
	{
	}

	void blendFuncSeparate(Enum src_rgb, Enum dst_rgb, Enum src_alpha, Enum dst_alpha)
	{
	}

	void blendFuncSeparateI(UInt buffer, Enum src_rgb, Enum dst_rgb, Enum src_alpha, Enum dst_alpha)
	{
	}

	void clampColor(Enum target, Enum clamp)
	{
	}

	void clipControl(Enum origin, Enum depth)
	{
	}

	void colorMask(Bool red, Bool green, Bool blue, Bool alpha)
	{
	}

	void colorMaskI(UInt index, Bool red, Bool green, Bool blue, Bool alpha)
	{
	}

	void cullFace(CullMode p_cull_mode)
	{
		glCullFace(static_cast<Enum>(p_cull_mode));
	}

	void depthFunc(Enum func)
	{
		glDepthFunc(func);
	}

	void depthMask(Bool flag)
	{
		glDepthMask(flag);
	}

	void depthRange(Double near_val, Double far_val)
	{
		glDepthRange(near_val, far_val);
	}

	void depthRangeF(Float near_val, Float far_val)
	{
		glDepthRangef(near_val, far_val);
	}

	void depthRangeArrayV(UInt first, UInt count, const Double *values)
	{
	}

	void depthRangeIndexed(UInt index, Double near_val, Double far_val)
	{
	}

	void disable(Capability p_capability)
	{
		glDisable(static_cast<Enum>(p_capability));
	}

	void disableI(Enum cap, UInt index)
	{
	}

	void enable(Capability p_capability)
	{
		glEnable(static_cast<Enum>(p_capability));
	}

	void enableI(Enum cap, UInt index)
	{
	}

	void frontFace(FaceOrientation p_face_orientation)
	{
		glFrontFace(static_cast<Enum>(p_face_orientation));
	}

	void getBoolV(Enum param_name, Bool *data)
	{
	}

	void getDoubleV(Enum param_name, Double *data)
	{
	}

	void getFloatV(Enum param_name, Float *data)
	{
	}

	void getIntegerV(Enum param_name, Int *data)
	{
	}

	void getInteger64V(Enum param_name, Int64 *data)
	{
	}

	void getBoolIv(Enum target, UInt index, Bool *data)
	{
	}

	void getIntegerIv(Enum target, UInt index, Int *data)
	{
	}

	void getFloatIv(Enum target, UInt index, Float *data)
	{
	}

	void getDoubleIv(Enum target, UInt index, Double *data)
	{
	}

	void getInteger64Iv(Enum target, UInt index, Int64 *data)
	{
	}

	Enum getError()
	{
		return glGetError();
	}

	void hint(Enum target, Enum mode)
	{
	}

	void isEnabled(Enum cap)
	{
	}

	void isEnabledI(Enum cap, UInt index)
	{
	}

	void lineWidth(Float width)
	{
	}

	void logicOp(Enum operation)
	{
	}

	void pixelStoreF(Enum pname, Float param)
	{
	}

	void pixelStoreI(Enum pname, Int param)
	{
	}

	void pointParameterF(Enum pname, Float param)
	{
	}

	void pointParameterI(Enum pname, Int param)
	{
	}

	void pointParameterFv(Enum pname, const Float *params)
	{
	}

	void pointParameterIv(Enum pname, const Int *params)
	{
	}

	void pointSize(Float size)
	{
	}

	void polygonMode(Enum face, Enum mode)
	{
	}

	void polygonOffset(Float factor, Float units)
	{
	}

	void sampleCoverage(Float value, Bool invert)
	{
	}

	void scissor(Int x, Int y, Int width, Int height)
	{
	}

	void scissorArrayV(UInt first, UInt count, const Int *v)
	{
	}

	void scissorIndexed(UInt index, Int x, Int y, SizeI width, SizeI height)
	{
	}

	void scissorIndexedV(UInt index, const Int *v)
	{
	}

	void stencilFunc(Enum func, Int ref, UInt mask)
	{
	}

	void stencilFuncSeparate(Enum face, Enum func, Int ref, UInt mask)
	{
	}

	void stencilMask(UInt mask)
	{
	}

	void stencilMaskSeparate(Enum face, UInt mask)
	{
	}

	void stencilOp(Enum sfail, Enum dpfail, Enum dppass)
	{
	}

	void stencilOpSeparate(Enum face, Enum sfail, Enum dpfail, Enum dppass)
	{
	}

	void viewport(Int x, Int y, SizeI width, SizeI height)
	{
		glViewport(x, y, width, height);
	}

	void viewportArrayV(UInt first, SizeI count, const Float *v)
	{
	}

	void viewportIndexedF(UInt index, Float x, Float y, Float width, Float height)
	{
	}

	void viewportIndexedFV(UInt index, const Float *v)
	{
	}
	#pragma endregion

	#pragma region Shaders

	void attachShader(UInt program, UInt shader)
	{
		glAttachShader(program, shader);
	}

	void bindAttribLocation(UInt program, UInt index, const Char *name)
	{
		glBindAttribLocation(program, index, name);
	}

	void bindFragDataLocation(UInt program, UInt colorNumber, const char *name)
	{
		glBindFragDataLocation(program, colorNumber, name);
	}

	void bindFragDataLocationIndexed(UInt program, UInt colorNumber, UInt index, const char *name)
	{
		glBindFragDataLocationIndexed(program, colorNumber, index, name);
	}

	void compileShader(UInt shader)
	{
		glCompileShader(shader);
	}

	UInt createProgram()
	{
		return glCreateProgram();
	}

	UInt createShader(ShaderStage p_shader_stage)
	{
		return glCreateShader(static_cast<Enum>(p_shader_stage));
	}

	UInt createShaderProgramV(ShaderStage p_shader_stage, SizeI count, const char **strings)
	{
		return glCreateShaderProgramv(static_cast<Enum>(p_shader_stage), count, strings);
	}

	void deleteProgram(UInt program)
	{
		glDeleteProgram(program);
	}

	void deleteShader(UInt shader)
	{
		glDeleteShader(shader);
	}

	void detachShader(UInt program, UInt shader)
	{
		glDetachShader(program, shader);
	}

	void getActiveAtomicCounterBufferiv(UInt program, UInt bufferIndex, Enum pname, Int *params)
	{
		glGetActiveAtomicCounterBufferiv(program, bufferIndex, pname, params);
	}

	void getActiveAttrib(UInt program, UInt index, SizeI bufSize, SizeI *length, Int *size, Enum *type, Char *name)
	{
		glGetActiveAttrib(program, index, bufSize, length, size, type, name);
	}

	void getActiveSubroutineName(UInt program, Enum shader_type, UInt index, SizeI buffer_size, SizeI *length, Char *name)
	{
		glGetActiveSubroutineName(program, shader_type, index, buffer_size, length, name);
	}

	void getActiveSubroutineUniformiv(UInt program, Enum shadertype, UInt index, Enum pname, Int *values)
	{
		glGetActiveSubroutineUniformiv(program, shadertype, index, pname, values);
	}

	void getActiveSubroutineUniformName(UInt program, Enum shader_type, UInt index, SizeI buffer_size, SizeI *length, Char *name)
	{
		glGetActiveSubroutineUniformName(program, shader_type, index, buffer_size, length, name);
	}

	void getActiveUniform(UInt program, UInt index, SizeI bufSize, SizeI *length, Int *size, Enum *type, Char *name)
	{
		glGetActiveUniform(program, index, bufSize, length, size, type, name);
	}

	void getActiveUniformBlockiv(UInt program, UInt uniformBlockIndex, Enum pname, Int *params)
	{
		glGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
	}

	void getActiveUniformBlockName(UInt program, UInt uniformBlockIndex, SizeI bufSize, SizeI *length, Char *uniformBlockName)
	{
		glGetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
	}

	void getActiveUniformName(UInt program, UInt uniformIndex, SizeI bufSize, SizeI *length, Char *uniformName)
	{
		glGetActiveUniformName(program, uniformIndex, bufSize, length, uniformName);
	}

	void getActiveUniformsiv(UInt program, SizeI uniformCount, const UInt *uniformIndices, Enum pname, Int *params)
	{
		glGetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
	}

	void getAttachedShaders(UInt program, SizeI maxCount, SizeI *count, UInt *shaders)
	{
		glGetAttachedShaders(program, maxCount, count, shaders);
	}

	Int getAttribLocation(UInt program, const Char *name)
	{
		return glGetAttribLocation(program, name);
	}

	Int getFragDataIndex(UInt program, const char *name)
	{
		return glGetFragDataIndex(program, name);
	}

	Int getFragDataLocation(UInt program, const char *name)
	{
		return glGetFragDataLocation(program, name);
	}

	void getProgramiv(UInt program, ProgramQuery p_query, Int *params)
	{
		glGetProgramiv(program, static_cast<Enum>(p_query), params);
	}

	void getProgramBinary(UInt program, SizeI buffer_size, SizeI *length, Enum *binaryFormat, void *binary)
	{
		glGetProgramBinary(program, buffer_size, length, binaryFormat, binary);
	}

	void getProgramInfoLog(UInt program, SizeI maxLength, SizeI *length, Char *infoLog)
	{
		glGetProgramInfoLog(program, maxLength, length, infoLog);
	}

	void getProgramResourceiv(UInt program, Enum programInterface, UInt index, SizeI propCount, const Enum *props, SizeI bufSize, SizeI *length, Int *params)
	{
		glGetProgramResourceiv(program, programInterface, index, propCount, props, bufSize, length, params);
	}

	UInt getProgramResourceIndex(UInt program, Enum programInterface, const char *name)
	{
		return glGetProgramResourceIndex(program, programInterface, name);
	}

	Int getProgramResourceLocation(UInt program, Enum programInterface, const char *name)
	{
		return glGetProgramResourceLocation(program, programInterface, name);
	}

	Int getProgramResourceLocationIndex(UInt program, Enum programInterface, const char *name)
	{
		return glGetProgramResourceLocationIndex(program, programInterface, name);
	}

	void getProgramResourceName(UInt program, Enum programInterface, UInt index, SizeI bufSize, SizeI *length, char *name)
	{
		glGetProgramResourceName(program, programInterface, index, bufSize, length, name);
	}

	void getProgramStageiv(UInt program, Enum shader_type, Enum pname, Int *values)
	{
		glGetProgramStageiv(program, shader_type, pname, values);
	}

	void getShaderiv(UInt shader, Enum pname, Int *params)
	{
		glGetShaderiv(shader, pname, params);
	}

	void getShaderInfoLog(UInt shader, SizeI maxLength, SizeI *length, Char *infoLog)
	{
		glGetShaderInfoLog(shader, maxLength, length, infoLog);
	}

	void getShaderPrecisionFormat(Enum shaderType, Enum precisionType, Int *range, Int *precision)
	{
		glGetShaderPrecisionFormat(shaderType, precisionType, range, precision);
	}

	void getShaderSource(UInt shader, SizeI bufSize, SizeI *length, Char *source)
	{
		glGetShaderSource(shader, bufSize, length, source);
	}

	UInt getSubroutineIndex(UInt program, Enum shader_type, const Char *name)
	{
		return glGetSubroutineIndex(program, shader_type, name);
	}

	Int getSubroutineUniformLocation(UInt program, Enum shader_type, const Char *name)
	{
		return glGetSubroutineUniformLocation(program, shader_type, name);
	}

	void getUniformfv(UInt program, Int location, Float *params)
	{
		glGetUniformfv(program, location, params);
	}

	void getUniformiv(UInt program, Int location, Int *params)
	{
		glGetUniformiv(program, location, params);
	}

	void getUniformuiv(UInt program, Int location, UInt *params)
	{
		glGetUniformuiv(program, location, params);
	}

	void getUniformdv(UInt program, Int location, Double *params)
	{
		glGetUniformdv(program, location, params);
	}

	void getnUniformfv(UInt program, Int location, SizeI bufSize, Float *params)
	{
		glGetnUniformfv(program, location, bufSize, params);
	}

	void getnUniformiv(UInt program, Int location, SizeI bufSize, Int *params)
	{
		glGetnUniformiv(program, location, bufSize, params);
	}

	void getnUniformuiv(UInt program, Int location, SizeI bufSize, UInt *params)
	{
		glGetnUniformuiv(program, location, bufSize, params);
	}

	void getnUniformdv(UInt program, Int location, SizeI bufSize, Double *params)
	{
		glGetnUniformdv(program, location, bufSize, params);
	}

	UInt getUniformBlockIndex(UInt program, const Char *uniformBlockName)
	{
		return glGetUniformBlockIndex(program, uniformBlockName);
	}

	void getUniformIndices(UInt program, SizeI uniformCount, const Char **uniformNames, UInt *uniformIndices)
	{
		glGetUniformIndices(program, uniformCount, uniformNames, uniformIndices);
	}

	Int getUniformLocation(UInt program, const Char *name)
	{
		return glGetUniformLocation(program, name);
	}

	void getUniformSubroutineuiv(Enum shader_type, Int location, UInt *values)
	{
		glGetUniformSubroutineuiv(shader_type, location, values);
	}

	Bool isProgram(UInt program)
	{
		return glIsProgram(program);
	}

	Bool isShader(UInt shader)
	{
		return glIsShader(shader);
	}

	void linkProgram(UInt program)
	{
		glLinkProgram(program);
	}

	void minSampleShading(Float value)
	{
		glMinSampleShading(value);
	}

	void programBinary(UInt program, ShaderBinaryFormat p_binary_format, const void *binary, SizeI length)
	{
		glProgramBinary(program, static_cast<Enum>(p_binary_format), binary, length);
	}

	void programParameteri(UInt program, Enum pname, Int value)
	{
		glProgramParameteri(program, pname, value);
	}

	void programUniform1f(UInt program, Int location, Float v0)
	{
		glProgramUniform1f(program, location, v0);
	}

	void programUniform2f(UInt program, Int location, Float v0, Float v1)
	{
		glProgramUniform2f(program, location, v0, v1);
	}

	void programUniform3f(UInt program, Int location, Float v0, Float v1, Float v2)
	{
		glProgramUniform3f(program, location, v0, v1, v2);
	}

	void programUniform4f(UInt program, Int location, Float v0, Float v1, Float v2, Float v3)
	{
		glProgramUniform4f(program, location, v0, v1, v2, v3);
	}

	void programUniform1i(UInt program, Int location, Int v0)
	{
		glProgramUniform1i(program, location, v0);
	}

	void programUniform2i(UInt program, Int location, Int v0, Int v1)
	{
		glProgramUniform2i(program, location, v0, v1);
	}

	void programUniform3i(UInt program, Int location, Int v0, Int v1, Int v2)
	{
		glProgramUniform3i(program, location, v0, v1, v2);
	}

	void programUniform4i(UInt program, Int location, Int v0, Int v1, Int v2, Int v3)
	{
		glProgramUniform4i(program, location, v0, v1, v2, v3);
	}

	void programUniform1ui(UInt program, Int location, UInt v0)
	{
		glProgramUniform1ui(program, location, v0);
	}

	void programUniform2ui(UInt program, Int location, UInt v0, UInt v1)
	{
		glProgramUniform2ui(program, location, v0, v1);
	}

	void programUniform3ui(UInt program, Int location, UInt v0, UInt v1, UInt v2)
	{
		glProgramUniform3ui(program, location, v0, v1, v2);
	}

	void programUniform4ui(UInt program, Int location, UInt v0, UInt v1, UInt v2, UInt v3)
	{
		glProgramUniform4ui(program, location, v0, v1, v2, v3);
	}

	void programUniform1fv(UInt program, Int location, SizeI count, const Float *p_value)
	{
		glProgramUniform1fv(program, location, count, p_value);
	}

	void programUniform2fv(UInt program, Int location, SizeI count, const Float *p_value)
	{
		glProgramUniform2fv(program, location, count, p_value);
	}

	void programUniform3fv(UInt program, Int location, SizeI count, const Float *p_value)
	{
		glProgramUniform3fv(program, location, count, p_value);
	}

	void programUniform4fv(UInt program, Int location, SizeI count, const Float *p_value)
	{
		glProgramUniform4fv(program, location, count, p_value);
	}

	void programUniform1iv(UInt program, Int location, SizeI count, const Int *value)
	{
		glProgramUniform1iv(program, location, count, value);
	}

	void programUniform2iv(UInt program, Int location, SizeI count, const Int *value)
	{
		glProgramUniform2iv(program, location, count, value);
	}

	void programUniform3iv(UInt program, Int location, SizeI count, const Int *value)
	{
		glProgramUniform3iv(program, location, count, value);
	}

	void programUniform4Iv(UInt program, Int location, SizeI count, const Int *value)
	{
		glProgramUniform4iv(program, location, count, value);
	}

	void programUniform1uiv(UInt program, Int location, SizeI count, const UInt *value)
	{
		glProgramUniform1uiv(program, location, count, value);
	}

	void programUniform2uiv(UInt program, Int location, SizeI count, const UInt *value)
	{
		glProgramUniform2uiv(program, location, count, value);
	}

	void programUniform3uiv(UInt program, Int location, SizeI count, const UInt *value)
	{
		glProgramUniform3uiv(program, location, count, value);
	}

	void programUniform4uiv(UInt program, Int location, SizeI count, const UInt *value)
	{
		glProgramUniform4uiv(program, location, count, value);
	}

	void programUniformMatrix2fv(UInt program, Int location, SizeI count, Bool transpose, const Float *p_value)
	{
		glProgramUniformMatrix2fv(program, location, count, transpose, p_value);
	}

	void programUniformMatrix3fv(UInt program, Int location, SizeI count, Bool transpose, const Float *p_value)
	{
		glProgramUniformMatrix3fv(program, location, count, transpose, p_value);
	}

	void programUniformMatrix4fv(UInt program, Int location, SizeI count, Bool transpose, const Float *p_value)
	{
		glProgramUniformMatrix4fv(program, location, count, transpose, p_value);
	}

	void programUniformMatrix2x3fv(UInt program, Int location, SizeI count, Bool transpose, const Float *p_value)
	{
		glProgramUniformMatrix2x3fv(program, location, count, transpose, p_value);
	}

	void programUniformMatrix3x2fv(UInt program, Int location, SizeI count, Bool transpose, const Float *p_value)
	{
		glProgramUniformMatrix3x2fv(program, location, count, transpose, p_value);
	}

	void programUniformMatrix2x4fv(UInt program, Int location, SizeI count, Bool transpose, const Float *p_value)
	{
		glProgramUniformMatrix2x4fv(program, location, count, transpose, p_value);
	}

	void programUniformMatrix4x2fv(UInt program, Int location, SizeI count, Bool transpose, const Float *p_value)
	{
		glProgramUniformMatrix4x2fv(program, location, count, transpose, p_value);
	}

	void programUniformMatrix3x4fv(UInt program, Int location, SizeI count, Bool transpose, const Float *p_value)
	{
		glProgramUniformMatrix3x4fv(program, location, count, transpose, p_value);
	}

	void programUniformMatrix4x3fv(UInt program, Int location, SizeI count, Bool transpose, const Float *p_value)
	{
		glProgramUniformMatrix4x3fv(program, location, count, transpose, p_value);
	}

	void releaseShaderCompiler()
	{
		glReleaseShaderCompiler();
	}

	void shaderBinary(SizeI count, const UInt *shaders, ShaderBinaryFormat p_binary_format, const void *binary, SizeI length)
	{
		glShaderBinary(count, shaders, static_cast<Enum>(p_binary_format), binary, length);
	}

	void shaderSource(UInt shader, SizeI count, const Char **string, const Int *length)
	{
		glShaderSource(shader, count, string, length);
	}

	void shaderStorageBlockBinding(UInt program, UInt storageBlockIndex, UInt storageBlockBinding)
	{
		glShaderStorageBlockBinding(program, storageBlockIndex, storageBlockBinding);
	}

	void specializeShader(UInt p_shader, const Char *p_entry_point, UInt p_num_specialization_constants, const UInt *p_constant_index, const UInt *p_constant_value)
	{
		glSpecializeShader(p_shader, p_entry_point, p_num_specialization_constants, p_constant_index, p_constant_value);
	}

	void uniform1f(Int location, Float v0)
	{
		glUniform1f(location, v0);
	}

	void uniform2f(Int location, Float v0, Float v1)
	{
		glUniform2f(location, v0, v1);
	}

	void uniform3f(Int location, Float v0, Float v1, Float v2)
	{
		glUniform3f(location, v0, v1, v2);
	}

	void uniform4f(Int location, Float v0, Float v1, Float v2, Float v3)
	{
		glUniform4f(location, v0, v1, v2, v3);
	}

	void uniform1d(Int location, Double v0)
	{
		glUniform1d(location, v0);
	}

	void uniform2d(Int location, Double v0, Double v1)
	{
		glUniform2d(location, v0, v1);
	}

	void uniform3d(Int location, Double v0, Double v1, Double v2)
	{
		glUniform3d(location, v0, v1, v2);
	}

	void uniform4d(Int location, Double v0, Double v1, Double v2, Double v3)
	{
		glUniform4d(location, v0, v1, v2, v3);
	}

	void uniform1i(Int location, Int v0)
	{
		glUniform1i(location, v0);
	}

	void uniform2i(Int location, Int v0, Int v1)
	{
		glUniform2i(location, v0, v1);
	}

	void uniform3i(Int location, Int v0, Int v1, Int v2)
	{
		glUniform3i(location, v0, v1, v2);
	}

	void uniform4i(Int location, Int v0, Int v1, Int v2, Int v3)
	{
		glUniform4i(location, v0, v1, v2, v3);
	}

	void uniform1ui(Int location, UInt v0)
	{
		glUniform1ui(location, v0);
	}

	void uniform2ui(Int location, UInt v0, UInt v1)
	{
		glUniform2ui(location, v0, v1);
	}

	void uniform3ui(Int location, UInt v0, UInt v1, UInt v2)
	{
		glUniform3ui(location, v0, v1, v2);
	}

	void uniform4ui(Int location, UInt v0, UInt v1, UInt v2, UInt v3)
	{
		glUniform4ui(location, v0, v1, v2, v3);
	}

	void uniform1fv(Int location, SizeI count, const Float *p_value)
	{
		glUniform1fv(location, count, p_value);
	}

	void uniform2fv(Int location, SizeI count, const Float *p_value)
	{
		glUniform2fv(location, count, p_value);
	}

	void uniform3fv(Int location, SizeI count, const Float *p_value)
	{
		glUniform3fv(location, count, p_value);
	}

	void uniform4fv(Int location, SizeI count, const Float *p_value)
	{
		glUniform4fv(location, count, p_value);
	}

	void uniform1dv(Int location, SizeI count, const Double *value)
	{
		glUniform1dv(location, count, value);
	}

	void uniform2dv(Int location, SizeI count, const Double *value)
	{
		glUniform2dv(location, count, value);
	}

	void uniform3dv(Int location, SizeI count, const Double *value)
	{
		glUniform3dv(location, count, value);
	}

	void uniform4dv(Int location, SizeI count, const Double *value)
	{
		glUniform4dv(location, count, value);
	}

	void uniform1iv(Int location, SizeI count, const Int *value)
	{
		glUniform1iv(location, count, value);
	}

	void uniform2iv(Int location, SizeI count, const Int *value)
	{
		glUniform2iv(location, count, value);
	}

	void uniform3iv(Int location, SizeI count, const Int *value)
	{
		glUniform3iv(location, count, value);
	}

	void uniform4iv(Int location, SizeI count, const Int *value)
	{
		glUniform4iv(location, count, value);
	}

	void uniform1uiv(Int location, SizeI count, const UInt *value)
	{
		glUniform1uiv(location, count, value);
	}

	void uniform2uiv(Int location, SizeI count, const UInt *value)
	{
		glUniform2uiv(location, count, value);
	}

	void uniform3uiv(Int location, SizeI count, const UInt *value)
	{
		glUniform3uiv(location, count, value);
	}

	void uniform4uiv(Int location, SizeI count, const UInt *value)
	{
		glUniform4uiv(location, count, value);
	}

	void uniformMatrix2fv(Int location, SizeI count, Bool transpose, const Float *p_value)
	{
		glUniformMatrix2fv(location, count, transpose, p_value);
	}

	void uniformMatrix3fv(Int location, SizeI count, Bool transpose, const Float *p_value)
	{
		glUniformMatrix3fv(location, count, transpose, p_value);
	}

	void uniformMatrix4fv(Int location, SizeI count, Bool transpose, const Float *p_value)
	{
		glUniformMatrix4fv(location, count, transpose, p_value);
	}

	void uniformMatrix2x3fv(Int location, SizeI count, Bool transpose, const Float *p_value)
	{
		glUniformMatrix2x3fv(location, count, transpose, p_value);
	}

	void uniformMatrix3x2fv(Int location, SizeI count, Bool transpose, const Float *p_value)
	{
		glUniformMatrix3x2fv(location, count, transpose, p_value);
	}

	void uniformMatrix2x4fv(Int location, SizeI count, Bool transpose, const Float *p_value)
	{
		glUniformMatrix2x4fv(location, count, transpose, p_value);
	}

	void uniformMatrix4x2fv(Int location, SizeI count, Bool transpose, const Float *p_value)
	{
		glUniformMatrix4x2fv(location, count, transpose, p_value);
	}

	void uniformMatrix3x4fv(Int location, SizeI count, Bool transpose, const Float *p_value)
	{
		glUniformMatrix3x4fv(location, count, transpose, p_value);
	}

	void uniformMatrix4x3fv(Int location, SizeI count, Bool transpose, const Float *p_value)
	{
		glUniformMatrix4x3fv(location, count, transpose, p_value);
	}

	void uniformBlockBinding(UInt program, UInt uniformBlockIndex, UInt uniformBlockBinding)
	{
		glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
	}

	void uniformSubroutinesuiv(Enum shader_type, SizeI count, const UInt *indices)
	{
		glUniformSubroutinesuiv(shader_type, count, indices);
	}

	void useProgram(UInt program)
	{
		glUseProgram(program);
	}

	void useProgramStages(UInt pipeline, Bitfield stages, UInt program)
	{
		glUseProgramStages(pipeline, stages, program);
	}

	void validateProgram(UInt program)
	{
		glValidateProgram(program);
	}
	#pragma endregion

	#pragma region Debug

	void debugMessageCallback(DebugCallbackProc callback, void *userParam)
	{
		glDebugMessageCallback(callback, userParam);
	}

	#pragma endregion
}
