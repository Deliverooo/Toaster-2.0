#pragma once

#include "gl_enums.hpp"
#include "platform.hpp"

// #include <glad/glad.h>

namespace gl
{
	// Source -> https://docs.gl/

	TST_API Int loadGL();

	#pragma region Rendering

	/*!
	 *
	 *
	 */
	TST_API void clear(BufferMaskFlags mask);

	/*!
	 *
	 *
	 */
	TST_API void clearBufferIv(Enum buffer, Int draw_buffer, const Int *value);

	/*!
	 *
	 *
	 */
	TST_API void clearBufferuiv(Enum buffer, Int draw_buffer, const UInt *value);

	/*!
	 *
	 *
	 */
	TST_API void clearBufferFv(Enum buffer, Int draw_buffer, const Float *p_value);

	/*!
	 *
	 *
	 */
	TST_API void clearBufferFi(Enum buffer, Int draw_buffer, Float depth, Int stencil);

	/*!
	 *
	 *
	 */
	TST_API void clearNamedFramebufferIv(UInt p_framebuffer, Enum buffer, Int draw_buffer, const Int *value);

	/*!
	 *
	 *
	 */
	TST_API void clearNamedFramebufferuiv(UInt p_framebuffer, Enum buffer, Int draw_buffer, const UInt *value);

	/*!
	 *
	 *
	 */
	TST_API void clearNamedFramebufferFv(UInt p_framebuffer, Enum buffer, Int draw_buffer, const Float *p_value);

	/*!
	 *
	 *
	 *
	 */
	TST_API void clearNamedFramebufferFi(UInt p_framebuffer, Enum buffer, Int draw_buffer, Float depth, Int stencil);

	/*!
	 *
	 *
	 */
	TST_API void clearColor(Float red, Float green, Float blue, Float alpha);

	/*!
	 *
	 *
	 */
	TST_API void clearDepth(Double depth);

	/*!
	 *
	 *
	 */
	TST_API void clearDepthF(Float depth);

	/*!
	 *
	 *
	 */
	TST_API void clearStencil(Int clear_value);

	/*!
	 *
	 *
	 */
	TST_API void drawBuffer(Enum colour_buffer);

	/*!
	 *
	 *
	 */
	TST_API void namedFramebufferDrawBuffer(UInt p_framebuffer, Enum buffer);

	/*!
	 *
	 *
	 */
	TST_API void finish();

	/*!
	 *
	 *
	 */
	TST_API void flush();

	/*!
	 *
	 *
	 */
	TST_API void readBuffer(Enum mode);

	/*!
	 *
	 *
	 */
	TST_API void namedFramebufferReadBuffer(UInt p_framebuffer, Enum mode);

	/*!
	 *
	 *
	 */
	TST_API void readPixels(Int x, Int y, Int width, Int height, Enum format, Enum type, Void *data);

	/*!
	 *
	 *
	 */
	TST_API void readnPixels(Int x, Int y, Int width, Int height, Enum format, Enum type, Int buffer_size, Void *data);

	#pragma endregion

	#pragma region Framebuffers

	TST_API void bindFramebuffer(FramebufferType p_target, UInt p_framebuffer);

	TST_API void bindRenderbuffer(FramebufferType p_target, UInt p_renderbuffer);

	TST_API void blitFramebuffer(Int              srcX0, Int srcY0, Int srcX1, Int srcY1, Int dstX0, Int dstY0, Int dstX1, Int dstY1, BufferMaskFlags p_mask,
								 TextureFiltering p_filter);

	TST_API void blitNamedFramebuffer(UInt read_framebuffer, UInt draw_framebuffer, Int    srcX0, Int srcY0, Int srcX1, Int srcY1, Int dstX0, Int dstY0, Int dstX1,
									  Int  dstY1, BufferMaskFlags p_mask, TextureFiltering p_filter);

	TST_API FramebufferStatus checkFramebufferStatus(FramebufferType p_target);

	TST_API FramebufferStatus checkNamedFramebufferStatus(UInt p_framebuffer, FramebufferType p_target);

	TST_API void createFramebuffers(SizeI count, UInt *p_buffers);

	TST_API void createRenderbuffers(SizeI count, UInt *p_renderbuffers);

	TST_API void deleteFramebuffers(SizeI count, UInt *p_framebuffers);

	TST_API void deleteRenderbuffers(SizeI count, UInt *p_renderbuffers);

	TST_API void drawBuffers(SizeI count, const Enum *p_buffers);

	TST_API void namedFramebufferDrawBuffers(UInt p_framebuffer, SizeI p_count, const Enum *p_buffers);

	TST_API void framebufferParameteri(FramebufferType p_target, FramebufferParameter p_framebuffer_parameter, Int p_param);

	TST_API void namedFramebufferParameteri(UInt p_framebuffer, FramebufferParameter p_framebuffer_parameter, Int p_param);

	TST_API void framebufferRenderbuffer(FramebufferType p_target, FramebufferAttachment p_attachment, RenderbufferType p_renderbuffer_target, UInt p_renderbuffer);

	TST_API void namedFramebufferRenderbuffer(UInt p_framebuffer, FramebufferAttachment p_attachment, RenderbufferType p_renderbuffer_target, UInt p_renderbuffer);

	TST_API void framebufferTexture(FramebufferType p_target, FramebufferAttachment p_attachment, UInt texture, Int level);

	TST_API void framebufferTexture1D(FramebufferType p_target, FramebufferAttachment p_attachment, TextureType p_tex_target, UInt texture, Int level);

	TST_API void framebufferTexture2D(FramebufferType p_target, FramebufferAttachment p_attachment, TextureType p_tex_target, UInt texture, Int level);

	TST_API void framebufferTexture3D(FramebufferType p_target, FramebufferAttachment p_attachment, TextureType p_tex_target, UInt texture, Int level, Int layer);

	TST_API void namedFramebufferTexture(UInt p_framebuffer, FramebufferAttachment p_attachment, UInt texture, Int level);

	TST_API void framebufferTextureLayer(Enum target, FramebufferAttachment p_attachment, UInt texture, Int level, Int layer);

	TST_API void namedFramebufferTextureLayer(UInt p_framebuffer, FramebufferAttachment p_attachment, UInt texture, Int level, Int layer);

	TST_API void genFramebuffers(SizeI count, UInt *p_buffers);

	TST_API void genRenderbuffers(SizeI count, UInt *p_renderbuffers);

	TST_API void generateMipmap(TextureType p_target);

	TST_API void generateTextureMipmap(UInt texture);

	TST_API void getFramebufferAttachmentParameterIv(Enum target, Enum attachment, Enum pname, Int *params);

	TST_API void getNamedFramebufferAttachmentParameterIv(UInt p_framebuffer, Enum attachment, Enum pname, Int *params);

	TST_API void getFramebufferParameterIv(Enum target, Enum pname, Int *params);

	TST_API void getNamedFramebufferParameterIv(UInt p_framebuffer, Enum pname, Int *params);

	TST_API void getRenderbufferParameterIv(Enum target, Enum pname, Int *params);

	TST_API void getNamedRenderbufferParameterIv(UInt p_renderbuffer, Enum pname, Int *params);

	TST_API void invalidateFramebuffer(Enum target, SizeI num_attachments, const Enum *attachments);

	TST_API void invalidateNamedFramebufferData(UInt p_framebuffer, SizeI num_attachments, const Enum *attachments);

	TST_API void invalidateSubFramebuffer(Enum target, SizeI num_attachments, const Enum *attachments, Int x, Int y, Int width, Int height);

	TST_API void invalidateNamedFramebufferSubData(UInt p_framebuffer, SizeI num_attachments, const Enum *attachments, Int x, Int y, Int width, Int height);

	TST_API bool isFramebuffer(UInt p_framebuffer);

	TST_API bool isRenderbuffer(UInt p_renderbuffer);

	TST_API void renderbufferStorage(Enum target, Format p_internal_format, SizeI width, SizeI height);

	TST_API void namedRenderbufferStorage(UInt p_renderbuffer, Format p_internal_format, SizeI width, SizeI height);

	TST_API void renderbufferStorageMultisample(Enum target, SizeI samples, Format p_internal_format, SizeI width, SizeI height);

	TST_API void namedRenderbufferStorageMultisample(UInt p_renderbuffer, SizeI samples, Format p_internal_format, SizeI width, SizeI height);

	TST_API void sampleMaskI(UInt index, Bitfield mask);

	#pragma endregion

	#pragma region Buffer Objects

	/**
	 *
	 **/
	TST_API void bindBuffer(BufferType p_target, UInt p_buffer);

	/**
	 *
	 **/
	TST_API void bindBufferBase(BufferType p_target, UInt p_index, UInt p_buffer);

	/**
	 *
	 **/
	TST_API void bindBufferRange(BufferType p_target, UInt p_index, UInt p_buffer, IntPtr p_offset, SizeIPtr p_size);

	/**
	 *
	 **/
	TST_API void bindBuffersBase(BufferType p_target, UInt first, SizeI count, const UInt *buffers);

	/**
	 *
	 **/
	TST_API void bindBuffersRange(BufferType p_target, UInt first, SizeI count, const UInt *buffers, IntPtr *offsets, SizeIPtr *sizes);

	/**
	 * Binds a buffer object to a specific binding point for front-face vertex data.
	 * This method sets up the connection between a vertex buffer object and a vertex array for rendering operations.
	 *
	 * @param binding_index The index of the binding point to which the buffer is bound.
	 * @param buffer The handle of the buffer object to bind.
	 * @param offset The offset in bytes from the start of the buffer object where vertex data begins.
	 * @param stride The byte stride between consecutive elements in the buffer.
	 */
	TST_API void bindglFrontFaceVertexBuffer(UInt binding_index, UInt buffer, IntPtr offset, SizeI stride);

	/**
	 *
	 **/
	TST_API void vertexArrayVertexBuffer(UInt vao, UInt binding_index, UInt buffer, IntPtr offset, SizeI stride);

	/**
	 *
	 **/
	TST_API void bindVertexBuffers(UInt first, SizeI count, const UInt *buffers, const IntPtr *offsets, const SizeI *strides);

	/**
	 *
	 **/
	TST_API void vertexArrayVertexBuffers(UInt vao, UInt first, SizeI count, const UInt *buffers, const IntPtr *offsets, const SizeI *strides);

	/**
	 * Creates a new data store for the buffer object currently bound to the target.
	 * Any pre-existing data store is deleted. The new data store is created with the specified size in bytes and usage.
	 * If data is not NULL, the data store is initialized with data from this pointer. In its initial state,
	 * the new data store is not mapped, it has a NULL mapped pointer, and its mapped access is GL_READ_WRITE.
	 **/
	TST_API void bufferData(BufferType p_target, SizeIPtr size, const Void *data, BufferUsage p_usage);

	/**
	 *
	 */
	TST_API void namedBufferData(UInt target, SizeIPtr size, const Void *data, BufferUsage p_usage);

	/**
	 *
	 **/
	TST_API void bufferStorage(BufferType p_target, SizeIPtr size, const Void *data, Bitfield flags);

	/**
	 *
	 **/
	TST_API void namedBufferStorage(UInt target, SizeIPtr size, const Void *data, Bitfield flags);

	/**
	 *
	 **/
	TST_API void bufferSubData(BufferType p_target, IntPtr offset, SizeIPtr size, const Void *data);

	/**
	 *
	 **/
	TST_API void namedBufferSubData(UInt target, IntPtr offset, SizeIPtr size, const Void *data);

	/**
	 *
	 **/
	TST_API void clearBufferData(BufferType p_target, Enum internal_format, Enum format, Enum type, const Void *data);

	/**
	 *
	 **/
	TST_API void clearNamedBufferData(UInt target, Enum internal_format, Enum format, Enum type, const Void *data);

	/**
	 *
	 **/
	TST_API void clearBufferSubData(BufferType p_target, Enum internal_format, IntPtr offset, SizeIPtr size, Enum format, Enum type, const Void *data);

	/**
	 *
	 **/
	TST_API void clearNamedBufferSubData(UInt target, Enum internal_format, IntPtr offset, SizeIPtr size, Enum format, Enum type, const Void *data);

	/**
	 *
	 **/
	TST_API void copyBufferSubData(BufferType p_read_target, BufferType p_write_target, IntPtr readOffset, IntPtr writeOffset, SizeIPtr size);

	/**
	 *
	 **/
	TST_API void copyNamedBufferSubData(UInt readBuffer, UInt writeBuffer, IntPtr readOffset, IntPtr writeOffset, SizeI size);

	/**
	 *
	 **/
	TST_API void createBuffers(SizeI count, UInt *buffers);

	/**
	 *
	 **/
	TST_API void createVertexArrays(SizeI count, UInt *arrays);

	/**
	 *
	 **/
	TST_API void deleteBuffers(SizeI count, const UInt *buffers);

	/**
	 *
	 **/
	TST_API void disableVertexAttribArray(UInt index);

	/**
	 *
	 **/
	TST_API void disableVertexArrayAttrib(UInt vao, UInt index);

	/**
	 *
	 **/
	TST_API void drawArrays(DrawMode p_mode, Int p_first, SizeI p_count);

	/**
	 *
	 **/
	TST_API void drawArraysIndirect(DrawMode p_mode, const Void *p_indirect);

	/**
	 *
	 **/
	TST_API void drawArraysInstanced(DrawMode p_mode, Int p_first, SizeI p_count, SizeI p_primitive_count);

	/**
	 *
	 **/
	TST_API void drawArraysInstancedBaseInstance(DrawMode p_mode, Int p_first, SizeI p_count, SizeI p_primitive_count, UInt p_base_instance);

	/**
	 *
	 **/
	TST_API void drawElements(DrawMode p_mode, SizeI p_count, DataType p_type, const Void *p_indices);

	/**
	 *
	 **/
	TST_API void drawElementsBaseVertex(DrawMode p_mode, SizeI p_count, DataType p_type, Void *p_indices, Int p_base_vertex);

	/**
	 *
	 **/
	TST_API void drawElementsIndirect(DrawMode p_mode, DataType p_type, const void *p_indirect);

	/**
	 *
	 **/
	TST_API void drawElementsInstanced(DrawMode p_mode, SizeI p_count, DataType p_type, const void *p_indices, SizeI p_primitive_count);

	/**
	 *
	 **/
	TST_API void drawElementsInstancedBaseInstance(DrawMode p_mode, SizeI p_count, DataType p_type, const void *p_indices, SizeI p_primitive_count, UInt p_base_instance);

	/**
	 *
	 **/
	TST_API void drawElementsInstancedBaseVertex(DrawMode p_mode, SizeI p_count, DataType p_type, Void *indices, SizeI p_primitive_count, Int p_base_vertex);

	/**
	 *
	 **/
	TST_API void drawElementsInstancedBaseVertexBaseInstance(DrawMode p_mode, SizeI p_count, DataType p_type, Void *p_indices, SizeI p_primitive_count, Int p_base_vertex,
															 UInt     p_base_instance);

	/**
	 *
	 **/
	TST_API void drawRangeElements(DrawMode p_mode, UInt p_start, UInt p_end, SizeI p_count, DataType p_type, const Void *indices);

	/**
	 *
	 **/
	TST_API void drawRangeElementsBaseVertex(DrawMode p_mode, UInt p_start, UInt p_end, SizeI p_count, DataType p_type, Void *p_indices, Int p_base_vertex);

	/**
	 *
	 **/
	TST_API void enableVertexAttribArray(UInt index);

	/**
	 *
	 **/
	TST_API void enableVertexArrayAttrib(UInt vao, UInt index);

	/**
	 *
	 **/
	TST_API void flushMappedBufferRange(Enum target, IntPtr offset, SizeIPtr length);

	/**
	 *
	 **/
	TST_API void flushMappedNamedBufferRange(UInt buffer, IntPtr offset, SizeIPtr length);

	/**
	 *
	 **/
	TST_API void genBuffers(SizeI n, UInt *buffers);

	/**
	 *
	 **/
	TST_API void getBufferParameterIv(Enum target, Enum value, Int *data);

	/**
	 *
	 **/
	TST_API void getBufferParameterI64V(Enum target, Enum value, Int64 *data);

	/**
	 *
	 **/
	TST_API void getNamedBufferParameterIv(UInt buffer, Enum pname, Int *params);

	/**
	 *
	 **/
	TST_API void getNamedBufferParameterI64V(UInt buffer, Enum pname, Int64 *params);

	/**
	 *
	 **/
	TST_API void getBufferPointerV(Enum target, Enum pname, Void **params);

	/**
	 *
	 **/
	TST_API void getNamedBufferPointerV(UInt buffer, Enum pname, void **params);

	/**
	 *
	 **/
	TST_API void getBufferSubData(Enum target, IntPtr offset, SizeIPtr size, Void *data);

	/**
	 *
	 **/
	TST_API void getNamedBufferSubData(UInt buffer, IntPtr offset, SizeI size, void *data);

	/**
	 *
	 **/
	TST_API void getVertexArrayIndexed64Iv(UInt vao, UInt index, Enum pname, Int64 *param);

	/**
	 *
	 **/
	TST_API void getVertexArrayIndexedIv(UInt vao, UInt index, Enum pname, Int *param);

	/**
	 *
	 **/
	TST_API void getVertexArrayIv(UInt vao, Enum pname, Int *param);

	/**
	 *
	 **/
	TST_API void getVertexAttribDv(UInt index, Enum pname, Double *params);

	/**
	 *
	 **/
	TST_API void getVertexAttribFv(UInt index, Enum pname, Float *params);

	/**
	 *
	 **/
	TST_API void getVertexAttribIv(UInt index, Enum pname, Int *params);

	/**
	 *
	 **/
	TST_API void getVertexAttribIiv(UInt index, Enum pname, Int *params);

	/**
	 *
	 **/
	TST_API void getVertexAttribIuiv(UInt index, Enum pname, UInt *params);

	/**
	 *
	 **/
	TST_API void getVertexAttribLdv(UInt index, Enum pname, Double *params);

	/**
	 *
	 **/
	TST_API void getVertexAttribPointerV(UInt index, Enum pname, Void **pointer);

	/**
	 *
	 **/
	TST_API void invalidateBufferData(UInt buffer);

	/**
	 *
	 **/
	TST_API void invalidateBufferSubData(UInt buffer, IntPtr offset, SizeIPtr length);

	/**
	 *
	 **/
	TST_API Bool isBuffer(UInt buffer);

	/**
	 *
	 **/
	TST_API Void *mapBuffer(Enum target, Enum access);

	/**
	 *
	 **/
	TST_API Void *mapNamedBuffer(UInt buffer, Enum access);

	/**
	 *
	 **/
	TST_API Void *mapBufferRange(Enum target, IntPtr offset, SizeIPtr length, Bitfield access);

	/**
	 *
	 **/
	TST_API Void *mapNamedBufferRange(UInt buffer, IntPtr offset, SizeIPtr length, Bitfield access);

	/**
	 *
	 **/
	TST_API void multiDrawArrays(Enum mode, const Int *first, const SizeI *count, SizeI draw_count);

	/**
	 *
	 **/
	TST_API void multiDrawArraysIndirect(Enum mode, const void *indirect, SizeI draw_count, SizeI stride);

	/**
	 *
	 **/
	TST_API void multiDrawElements(Enum mode, const SizeI *count, Enum type, const Void *const *indices, SizeI draw_count);

	/**
	 *
	 **/
	TST_API void multiDrawElementsBaseVertex(Enum mode, const SizeI *count, Enum type, const Void *const *indices, SizeI draw_count, const Int *base_vertex);

	/**
	 *
	 **/
	TST_API void multiDrawElementsIndirect(Enum mode, Enum type, const void *indirect, SizeI draw_count, SizeI stride);

	/**
	 *
	 **/
	TST_API void patchParameterI(Enum pname, Int value);

	/**
	 *
	 **/
	TST_API void patchParameterFv(Enum pname, const Float *p_values);

	/**
	 *
	 **/
	TST_API void primitiveRestartIndex(UInt index);

	/**
	 *
	 **/
	TST_API void provokingVertex(Enum provokeMode);

	/**
	 *
	 **/
	TST_API Bool unmapBuffer(Enum target);

	/**
	 *
	 **/
	TST_API Bool unmapNamedBuffer(UInt buffer);

	/**
	 *
	 **/
	TST_API void vertexArrayElementBuffer(UInt vao, UInt buffer);

	TST_API void vertexAttrib1f(UInt index, Float v0);

	TST_API void vertexAttrib1s(UInt index, Short v0);

	TST_API void vertexAttrib1d(UInt index, Double v0);

	TST_API void vertexAttribI1i(UInt index, Int v0);

	TST_API void vertexAttribI1ui(UInt index, UInt v0);

	TST_API void vertexAttrib2f(UInt index, Float v0, Float v1);

	TST_API void vertexAttrib2s(UInt index, Short v0, Short v1);

	TST_API void vertexAttrib2d(UInt index, Double v0, Double v1);

	TST_API void vertexAttribI2i(UInt index, Int v0, Int v1);

	TST_API void vertexAttribI2ui(UInt index, UInt v0, UInt v1);

	TST_API void vertexAttrib3f(UInt index, Float v0, Float v1, Float v2);

	TST_API void vertexAttrib3s(UInt index, Short v0, Short v1, Short v2);

	TST_API void vertexAttrib3d(UInt index, Double v0, Double v1, Double v2);

	TST_API void vertexAttribI3i(UInt index, Int v0, Int v1, Int v2);

	TST_API void vertexAttribI3ui(UInt index, UInt v0, UInt v1, UInt v2);

	TST_API void vertexAttrib4f(UInt index, Float v0, Float v1, Float v2, Float v3);

	TST_API void vertexAttrib4s(UInt index, Short v0, Short v1, Short v2, Short v3);

	TST_API void vertexAttrib4d(UInt index, Double v0, Double v1, Double v2, Double v3);

	TST_API void vertexAttrib4Nub(UInt index, UByte v0, UByte v1, UByte v2, UByte v3);

	TST_API void vertexAttribI4i(UInt index, Int v0, Int v1, Int v2, Int v3);

	TST_API void vertexAttribI4ui(UInt index, UInt v0, UInt v1, UInt v2, UInt v3);

	TST_API void vertexAttribL1d(UInt index, Double v0);

	TST_API void vertexAttribL2d(UInt index, Double v0, Double v1);

	TST_API void vertexAttribL3d(UInt index, Double v0, Double v1, Double v2);

	TST_API void vertexAttribL4d(UInt index, Double v0, Double v1, Double v2, Double v3);

	TST_API void vertexAttrib1fv(UInt index, const Float *v);

	TST_API void vertexAttrib1sv(UInt index, const Short *v);

	TST_API void vertexAttrib1dv(UInt index, const Double *v);

	TST_API void vertexAttribI1iv(UInt index, const Int *v);

	TST_API void vertexAttribI1uiv(UInt index, const UInt *v);

	TST_API void vertexAttrib2fv(UInt index, const Float *v);

	TST_API void vertexAttrib2sv(UInt index, const Short *v);

	TST_API void vertexAttrib2dv(UInt index, const Double *v);

	TST_API void vertexAttribI2iv(UInt index, const Int *v);

	TST_API void vertexAttribI2uiv(UInt index, const UInt *v);

	TST_API void vertexAttrib3fv(UInt index, const Float *v);

	TST_API void vertexAttrib3sv(UInt index, const Short *v);

	TST_API void vertexAttrib3dv(UInt index, const Double *v);

	TST_API void vertexAttribI3iv(UInt index, const Int *v);

	TST_API void vertexAttribI3uiv(UInt index, const UInt *v);

	TST_API void vertexAttrib4fv(UInt index, const Float *v);

	TST_API void vertexAttrib4sv(UInt index, const Short *v);

	TST_API void vertexAttrib4dv(UInt index, const Double *v);

	TST_API void vertexAttrib4iv(UInt index, const Int *v);

	TST_API void vertexAttrib4bv(UInt index, const Byte *v);

	TST_API void vertexAttrib4ubv(UInt index, const UByte *v);

	TST_API void vertexAttrib4usv(UInt index, const UShort *v);

	TST_API void vertexAttrib4uiv(UInt index, const UInt *v);

	TST_API void vertexAttrib4Nbv(UInt index, const Byte *v);

	TST_API void vertexAttrib4Nsv(UInt index, const Short *v);

	TST_API void vertexAttrib4Niv(UInt index, const Int *v);

	TST_API void vertexAttrib4Nubv(UInt index, const UByte *v);

	TST_API void vertexAttrib4Nusv(UInt index, const UShort *v);

	TST_API void vertexAttrib4Nuiv(UInt index, const UInt *v);

	TST_API void vertexAttribI4bv(UInt index, const Byte *v);

	TST_API void vertexAttribI4ubv(UInt index, const UByte *v);

	TST_API void vertexAttribI4sv(UInt index, const Short *v);

	TST_API void vertexAttribI4usv(UInt index, const UShort *v);

	TST_API void vertexAttribI4iv(UInt index, const Int *v);

	TST_API void vertexAttribI4uiv(UInt index, const UInt *v);

	TST_API void vertexAttribL1dv(UInt index, const Double *v);

	TST_API void vertexAttribL2dv(UInt index, const Double *v);

	TST_API void vertexAttribL3dv(UInt index, const Double *v);

	TST_API void vertexAttribL4dv(UInt index, const Double *v);

	TST_API void vertexAttribP1ui(UInt index, Enum type, Bool normalized, UInt value);

	TST_API void vertexAttribP2ui(UInt index, Enum type, Bool normalized, UInt value);

	TST_API void vertexAttribP3ui(UInt index, Enum type, Bool normalized, UInt value);

	TST_API void vertexAttribP4ui(UInt index, Enum type, Bool normalized, UInt value);

	/**
	 *
	 **/
	TST_API void vertexAttribBinding(UInt attrib_index, UInt binding_index);

	/**
	 *
	 **/
	TST_API void vertexArrayAttribBinding(UInt vao, UInt attrib_index, UInt binding_index);

	/**
	 *
	 **/
	TST_API void vertexAttribDivisor(UInt index, UInt divisor);

	TST_API void vertexAttribFormat(UInt attrib_index, Int size, DataType p_type, Bool normalized, UInt relative_offset);

	TST_API void vertexAttribIFormat(UInt attrib_index, Int size, DataType p_type, UInt relative_offset);

	TST_API void vertexAttribLFormat(UInt attrib_index, Int size, DataType p_type, UInt relative_offset);

	TST_API void vertexArrayAttribFormat(UInt vao, UInt attrib_index, Int size, DataType p_type, Bool normalized, UInt relative_offset);

	TST_API void vertexArrayAttribIFormat(UInt vao, UInt attrib_index, Int size, DataType p_type, UInt relative_offset);

	TST_API void vertexArrayAttribLFormat(UInt vao, UInt attrib_index, Int size, DataType p_type, UInt relative_offset);

	TST_API void vertexAttribPointer(UInt index, Int size, DataType p_type, Bool normalized, SizeI stride, const Void *pointer);

	TST_API void vertexAttribIPointer(UInt index, Int size, DataType p_type, SizeI stride, const Void *pointer);

	TST_API void vertexAttribLPointer(UInt index, Int size, DataType p_type, SizeI stride, const Void *pointer);

	/**
	 *
	 **/
	TST_API void vertexBindingDivisor(UInt binding_index, UInt divisor);

	/**
	 *
	 **/
	TST_API void vertexArrayBindingDivisor(UInt vao, UInt binding_index, UInt divisor);

	#pragma endregion

	#pragma region State Management

	TST_API void blendColor(Float red, Float green, Float blue, Float alpha);

	TST_API void blendEquation(Enum mode);

	TST_API void blendEquationI(UInt buffer, Enum mode);

	TST_API void blendEquationSeparate(Enum mode_RGB, Enum mode_alpha);

	TST_API void blendEquationSeparateI(UInt buffer, Enum mode_RGB, Enum mode_alpha);

	TST_API void blendFunc(BlendFunc p_s_factor, BlendFunc p_d_factor);

	TST_API void blendFuncI(UInt buffer, Enum s_factor, Enum d_factor);

	TST_API void blendFuncSeparate(Enum src_rgb, Enum dst_rgb, Enum src_alpha, Enum dst_alpha);

	TST_API void blendFuncSeparateI(UInt buffer, Enum src_rgb, Enum dst_rgb, Enum src_alpha, Enum dst_alpha);

	TST_API void clampColor(Enum target, Enum clamp);

	TST_API void clipControl(Enum origin, Enum depth);

	TST_API void colorMask(Bool red, Bool green, Bool blue, Bool alpha);

	TST_API void colorMaskI(UInt index, Bool red, Bool green, Bool blue, Bool alpha);

	TST_API void cullFace(CullMode p_cull_mode);

	TST_API void depthFunc(Enum func);

	TST_API void depthMask(Bool flag);

	TST_API void depthRange(Double near_val, Double far_val);

	TST_API void depthRangeF(Float near_val, Float far_val);

	TST_API void depthRangeArrayV(UInt first, UInt count, const Double *values);

	TST_API void depthRangeIndexed(UInt index, Double near_val, Double far_val);

	TST_API void disable(Capability p_capability);

	TST_API void disableI(Enum cap, UInt index);

	TST_API void enable(Capability p_capability);

	TST_API void enableI(Enum cap, UInt index);

	TST_API void frontFace(FaceOrientation p_face_orientation);

	TST_API void getBoolV(Enum param_name, Bool *data);

	TST_API void getDoubleV(Enum param_name, Double *data);

	TST_API void getFloatV(Enum param_name, Float *data);

	TST_API void getIntegerV(Enum param_name, Int *data);

	TST_API void getInteger64V(Enum param_name, Int64 *data);

	TST_API void getBoolIv(Enum target, UInt index, Bool *data);

	TST_API void getIntegerIv(Enum target, UInt index, Int *data);

	TST_API void getFloatIv(Enum target, UInt index, Float *data);

	TST_API void getDoubleIv(Enum target, UInt index, Double *data);

	TST_API void getInteger64Iv(Enum target, UInt index, Int64 *data);

	TST_API Enum getError();

	TST_API void hint(Enum target, Enum mode);

	TST_API void isEnabled(Enum cap);

	TST_API void isEnabledI(Enum cap, UInt index);

	TST_API void lineWidth(Float width);

	TST_API void logicOp(Enum operation);

	TST_API void pixelStoreF(Enum pname, Float param);

	TST_API void pixelStoreI(Enum pname, Int param);

	TST_API void pointParameterF(Enum pname, Float param);

	TST_API void pointParameterI(Enum pname, Int param);

	TST_API void pointParameterFv(Enum pname, const Float *params);

	TST_API void pointParameterIv(Enum pname, const Int *params);

	TST_API void pointSize(Float size);

	TST_API void polygonMode(Enum face, Enum mode);

	TST_API void polygonOffset(Float factor, Float units);

	TST_API void sampleCoverage(Float value, Bool invert);

	TST_API void scissor(Int x, Int y, Int width, Int height);

	TST_API void scissorArrayV(UInt first, UInt count, const Int *v);

	TST_API void scissorIndexed(UInt index, Int x, Int y, SizeI width, SizeI height);

	TST_API void scissorIndexedV(UInt index, const Int *v);

	TST_API void stencilFunc(Enum func, Int ref, UInt mask);

	TST_API void stencilFuncSeparate(Enum face, Enum func, Int ref, UInt mask);

	TST_API void stencilMask(UInt mask);

	TST_API void stencilMaskSeparate(Enum face, UInt mask);

	TST_API void stencilOp(Enum sfail, Enum dpfail, Enum dppass);

	TST_API void stencilOpSeparate(Enum face, Enum sfail, Enum dpfail, Enum dppass);

	TST_API void viewport(Int x, Int y, SizeI width, SizeI height);

	TST_API void viewportArrayV(UInt first, SizeI count, const Float *v);

	TST_API void viewportIndexedF(UInt index, Float x, Float y, Float width, Float height);

	TST_API void viewportIndexedFV(UInt index, const Float *v);

	#pragma endregion

	#pragma region Vertex Array Objects

	TST_API void bindVertexArray(UInt array);

	TST_API void deleteVertexArrays(SizeI n, const UInt *arrays);

	TST_API void genVertexArrays(SizeI n, UInt *arrays);

	TST_API Bool isVertexArray(UInt array);

	#pragma endregion

	#pragma region Textures

	/*!
	 * @brief selects which texture unit subsequent texture state calls will affect.
	 * The number of texture units an implementation supports is implementation dependent, but must be at least 80.
	 * @param p_texture Specifies which texture unit to make active.
	 */
	TST_API void activeTexture(TextureUnit p_texture);

	TST_API void bindImageTexture(UInt unit, UInt texture, Int level, Bool layered, Int layer, Enum access, Format p_format);

	TST_API void bindImageTextures(UInt first, SizeI count, const UInt *textures);

	TST_API void bindTexture(TextureType p_target, UInt texture);

	TST_API void bindTextureUnit(UInt p_texture_unit, UInt texture);

	TST_API void bindTextures(UInt first, SizeI count, const UInt *textures);

	TST_API void clearTexImage(UInt texture, Int level, Format p_format, DataType p_type, const void *data);

	TST_API void clearTexSubImage(UInt     texture, Int        level, Int x_offset, Int y_offset, Int z_offset, SizeI width, SizeI height, SizeI depth, Format p_format,
								  DataType p_type, const void *data);

	TST_API void compressedTexImage1D(TextureType p_target, Int level, Format p_internal_format, SizeI width, Int border, SizeI imageSize, const Void *data);

	TST_API void compressedTexImage2D(TextureType p_target, Int level, Format p_internal_format, SizeI width, SizeI height, Int border, SizeI imageSize,
									  const Void *data);

	TST_API void compressedTexImage3D(TextureType p_target, Int level, Format p_internal_format, SizeI width, SizeI height, SizeI depth, Int border, SizeI imageSize,
									  const Void *data);

	TST_API void compressedTexSubImage1D(TextureType p_target, Int level, Int x_offset, SizeI width, Format p_format, SizeI imageSize, const Void *data);

	TST_API void compressedTextureSubImage1D(UInt texture, Int level, Int x_offset, SizeI width, Format p_format, SizeI imageSize, const void *data);

	TST_API void compressedTexSubImage2D(TextureType p_target, Int level, Int x_offset, Int y_offset, SizeI width, SizeI height, Format p_format, SizeI imageSize,
										 const Void *data);

	TST_API void compressedTextureSubImage2D(UInt        texture, Int level, Int x_offset, Int y_offset, SizeI width, SizeI height, Format p_format, SizeI imageSize,
											 const void *data);

	TST_API void compressedTexSubImage3D(TextureType p_target, Int   level, Int             x_offset, Int y_offset, Int z_offset, SizeI width, SizeI height, SizeI depth,
										 Format      p_format, SizeI imageSize, const Void *data);

	TST_API void compressedTextureSubImage3D(UInt  texture, Int level, Int x_offset, Int y_offset, Int z_offset, SizeI width, SizeI height, SizeI depth, Format p_format,
											 SizeI imageSize, const void *data);

	TST_API void copyImageSubData(UInt srcName, TextureType p_src_target, Int srcLevel, Int srcX, Int   srcY, Int       srcZ, UInt dstName, TextureType p_dst_target,
								  Int  dstLevel, Int        dstX, Int         dstY, Int     dstZ, SizeI srcWidth, SizeI srcHeight, SizeI srcDepth);

	TST_API void copyTexImage1D(TextureType p_target, Int level, Format p_internal_format, Int x, Int y, SizeI width, Int border);

	TST_API void copyTexImage2D(TextureType p_target, Int level, Format p_internal_format, Int x, Int y, SizeI width, SizeI height, Int border);

	TST_API void copyTexSubImage1D(TextureType p_target, Int level, Int x_offset, Int x, Int y, SizeI width);

	TST_API void copyTextureSubImage1D(UInt texture, Int level, Int x_offset, Int x, Int y, SizeI width);

	TST_API void copyTexSubImage2D(TextureType p_target, Int level, Int x_offset, Int y_offset, Int x, Int y, SizeI width, SizeI height);

	TST_API void copyTextureSubImage2D(UInt texture, Int level, Int x_offset, Int y_offset, Int x, Int y, SizeI width, SizeI height);

	TST_API void copyTexSubImage3D(TextureType p_target, Int level, Int x_offset, Int y_offset, Int z_offset, Int x, Int y, SizeI width, SizeI height);

	TST_API void copyTextureSubImage3D(UInt texture, Int level, Int x_offset, Int y_offset, Int z_offset, Int x, Int y, SizeI width, SizeI height);

	TST_API void createTextures(TextureType p_target, SizeI n, UInt *textures);

	TST_API void deleteTextures(SizeI n, const UInt *textures);

	TST_API void genTextures(SizeI n, UInt *textures);

	TST_API void getCompressedTexImage(TextureType p_target, Int level, Void *pixels);

	TST_API void getnCompressedTexImage(TextureType p_target, Int level, SizeI bufSize, void *pixels);

	TST_API void getCompressedTextureImage(UInt texture, Int level, SizeI bufSize, void *pixels);

	TST_API void getCompressedTextureSubImage(UInt  texture, Int level, Int x_offset, Int y_offset, Int z_offset, SizeI width, SizeI height, SizeI depth, SizeI bufSize,
											  void *pixels);

	TST_API void getTexImage(TextureType p_target, Int level, Format p_format, DataType p_type, Void *pixels);

	TST_API void getnTexImage(TextureType p_target, Int level, Format p_format, DataType p_type, SizeI bufSize, void *pixels);

	TST_API void getTextureImage(UInt texture, Int level, Format p_format, DataType p_type, SizeI bufSize, void *pixels);

	TST_API void getTexLevelParameterFv(TextureType p_target, Int level, Enum pname, Float *params);

	TST_API void getTexLevelParameterIv(TextureType p_target, Int level, Enum pname, Int *params);

	TST_API void getTextureLevelParameterFv(UInt texture, Int level, Enum pname, Float *params);

	TST_API void getTextureLevelParameterIv(UInt texture, Int level, Enum pname, Int *params);

	TST_API void getTexParameterFv(TextureType p_target, Enum pname, Float *params);

	TST_API void getTexParameterIv(TextureType p_target, Enum pname, Int *params);

	TST_API void getTexParameterIiv(TextureType p_target, Enum pname, Int *params);

	TST_API void getTexParameterIuiv(TextureType p_target, Enum pname, UInt *params);

	TST_API void getTextureParameterFv(UInt texture, Enum pname, Float *params);

	TST_API void getTextureParameterIv(UInt texture, Enum pname, Int *params);

	TST_API void getTextureParameterIiv(UInt texture, Enum pname, Int *params);

	TST_API void getTextureParameterIuiv(UInt texture, Enum pname, UInt *params);

	TST_API void getTextureSubImage(UInt     texture, Int  level, Int     x_offset, Int y_offset, Int z_offset, SizeI width, SizeI height, SizeI depth, Format p_format,
									DataType p_type, SizeI bufSize, void *pixels);

	TST_API void invalidateTexImage(UInt texture, Int level);

	TST_API void invalidateTexSubImage(UInt texture, Int level, Int x_offset, Int y_offset, Int z_offset, SizeI width, SizeI height, SizeI depth);

	TST_API Bool isTexture(UInt texture);

	TST_API void texBuffer(TextureType p_target, Format p_internal_format, UInt buffer);

	TST_API void textureBuffer(UInt texture, Format p_internal_format, UInt buffer);

	TST_API void texBufferRange(TextureType p_target, Format p_internal_format, UInt buffer, IntPtr offset, SizeIPtr size);

	TST_API void textureBufferRange(UInt texture, Format p_internal_format, UInt buffer, IntPtr offset, SizeI size);

	TST_API void texImage1D(TextureType p_target, Int level, Format p_internal_format, SizeI width, Int border, Format p_format, DataType p_type, const Void *data);

	TST_API void texImage2D(TextureType p_target, Int level, Format p_internal_format, SizeI width, SizeI height, Int border, Format p_format, DataType p_type,
							const Void *data);

	TST_API void texImage2DMultisample(TextureType p_target, SizeI samples, Format p_internal_format, SizeI width, SizeI height, Bool fixed_sample_locations);

	TST_API void texImage3D(TextureType p_target, Int       level, Format p_internal_format, SizeI width, SizeI height, SizeI depth, Int border, Format p_format,
							DataType    p_type, const Void *data);

	TST_API void texImage3DMultisample(TextureType p_target, SizeI samples, Format p_internal_format, SizeI width, SizeI height, SizeI depth,
									   Bool        fixed_sample_locations);

	TST_API void texParameterf(TextureType p_target, SamplerParameter p_sampler_parameter, Float param);

	TST_API void texParameteri(TextureType p_target, SamplerParameter p_sampler_parameter, Int param);

	TST_API void textureParameterf(UInt texture, SamplerParameter p_sampler_parameter, Float param);

	TST_API void textureParameteri(UInt texture, SamplerParameter p_sampler_parameter, Int param);

	TST_API void texParameterFv(TextureType p_target, SamplerParameter p_sampler_parameter, const Float *params);

	TST_API void texParameterIv(TextureType p_target, SamplerParameter p_sampler_parameter, const Int *params);

	TST_API void texParameterIiv(TextureType p_target, SamplerParameter p_sampler_parameter, const Int *params);

	TST_API void texParameterIuiv(TextureType p_target, SamplerParameter p_sampler_parameter, const UInt *params);

	TST_API void textureParameterFv(UInt texture, SamplerParameter p_sampler_parameter, const Float *param_texture);

	TST_API void textureParameterIv(UInt texture, SamplerParameter p_sampler_parameter, const Int *param);

	TST_API void textureParameterIiv(UInt texture, SamplerParameter p_sampler_parameter, const Int *params);

	TST_API void textureParameterIuiv(UInt texture, SamplerParameter p_sampler_parameter, const UInt *params);

	TST_API void texStorage1D(TextureType p_target, SizeI levels, Format p_internal_format, SizeI width);

	TST_API void textureStorage1D(UInt texture, SizeI levels, Format p_internal_format, SizeI width);

	TST_API void texStorage2D(TextureType p_target, SizeI levels, Format p_internal_format, SizeI width, SizeI height);

	TST_API void textureStorage2D(UInt texture, SizeI levels, Format p_internal_format, SizeI width, SizeI height);

	TST_API void texStorage2DMultisample(TextureType p_target, SizeI samples, Format p_internal_format, SizeI width, SizeI height, Bool fixed_sample_locations);

	TST_API void textureStorage2DMultisample(UInt texture, SizeI samples, Format p_internal_format, SizeI width, SizeI height, Bool fixed_sample_locations);

	TST_API void texStorage3D(TextureType p_target, SizeI levels, Format p_internal_format, SizeI width, SizeI height, SizeI depth);

	TST_API void textureStorage3D(UInt texture, SizeI levels, Format p_internal_format, SizeI width, SizeI height, SizeI depth);

	TST_API void texStorage3DMultisample(TextureType p_target, SizeI samples, Format p_internal_format, SizeI width, SizeI height, SizeI depth,
										 Bool        fixed_sample_locations);

	TST_API void textureStorage3DMultisample(UInt texture, SizeI samples, Format p_internal_format, SizeI width, SizeI height, SizeI depth, Bool fixed_sample_locations);

	TST_API void texSubImage1D(TextureType p_target, Int level, Int x_offset, SizeI width, Format p_format, DataType p_type, const Void *pixels);

	TST_API void textureSubImage1D(UInt texture, Int level, Int x_offset, SizeI width, Format p_format, DataType p_type, const void *pixels);

	TST_API void texSubImage2D(TextureType p_target, Int level, Int x_offset, Int y_offset, SizeI width, SizeI height, Format p_format, DataType p_type,
							   const Void *pixels);

	TST_API void textureSubImage2D(UInt texture, Int level, Int x_offset, Int y_offset, SizeI width, SizeI height, Format p_format, DataType p_type, const void *pixels);

	TST_API void texSubImage3D(TextureType p_target, Int       level, Int x_offset, Int y_offset, Int z_offset, SizeI width, SizeI height, SizeI depth, Format p_format,
							   DataType    p_type, const Void *pixels);

	TST_API void textureSubImage3D(UInt     texture, Int        level, Int x_offset, Int y_offset, Int z_offset, SizeI width, SizeI height, SizeI depth, Format p_format,
								   DataType p_type, const void *pixels);

	TST_API void textureView(UInt p_texture, TextureType p_target, UInt p_origin_texture, Format p_internal_format, UInt p_min_level, UInt p_num_levels, UInt p_min_layer, UInt p_num_layers);

	#pragma endregion

	#pragma region Shaders

	TST_API void attachShader(UInt program, UInt shader);

	TST_API void bindAttribLocation(UInt program, UInt index, const Char *name);

	TST_API void bindFragDataLocation(UInt program, UInt colorNumber, const char *name);

	TST_API void bindFragDataLocationIndexed(UInt program, UInt colorNumber, UInt index, const char *name);

	TST_API void compileShader(UInt shader);

	TST_API UInt createProgram();

	TST_API UInt createShader(ShaderStage p_shader_stage);

	TST_API UInt createShaderProgramV(ShaderStage p_shader_stage, SizeI count, const char **strings);

	TST_API void deleteProgram(UInt program);

	TST_API void deleteShader(UInt shader);

	TST_API void detachShader(UInt program, UInt shader);

	TST_API void getActiveAtomicCounterBufferiv(UInt program, UInt bufferIndex, Enum pname, Int *params);

	TST_API void getActiveAttrib(UInt program, UInt index, SizeI bufSize, SizeI *length, Int *size, Enum *type, Char *name);

	TST_API void getActiveSubroutineName(UInt program, Enum shader_type, UInt index, SizeI buffer_size, SizeI *length, Char *name);

	TST_API void getActiveSubroutineUniformiv(UInt program, Enum shader_type, UInt index, Enum pname, Int *values);

	TST_API void getActiveSubroutineUniformName(UInt program, Enum shader_type, UInt index, SizeI buffer_size, SizeI *length, Char *name);

	TST_API void getActiveUniform(UInt program, UInt index, SizeI bufSize, SizeI *length, Int *size, Enum *type, Char *name);

	TST_API void getActiveUniformBlockIv(UInt program, UInt uniformBlockIndex, Enum pname, Int *params);

	TST_API void getActiveUniformBlockName(UInt program, UInt uniformBlockIndex, SizeI bufSize, SizeI *length, Char *uniformBlockName);

	TST_API void getActiveUniformName(UInt program, UInt uniformIndex, SizeI bufSize, SizeI *length, Char *uniformName);

	TST_API void getActiveUniformsIv(UInt program, SizeI uniformCount, const UInt *uniformIndices, Enum pname, Int *params);

	TST_API void getAttachedShaders(UInt program, SizeI maxCount, SizeI *count, UInt *shaders);

	TST_API Int getAttribLocation(UInt program, const Char *name);

	TST_API Int getFragDataIndex(UInt program, const char *name);

	TST_API Int getFragDataLocation(UInt program, const char *name);

	TST_API void getProgramiv(UInt program, ProgramQuery p_query, Int *params);

	TST_API void getProgramBinary(UInt program, SizeI buffer_size, SizeI *length, Enum *binaryFormat, void *binary);

	TST_API void getProgramInfoLog(UInt program, SizeI maxLength, SizeI *length, Char *infoLog);

	TST_API void getProgramResourceiv(UInt program, Enum programInterface, UInt index, SizeI propCount, const Enum *props, SizeI bufSize, SizeI *length, Int *params);

	TST_API UInt getProgramResourceIndex(UInt program, Enum programInterface, const char *name);

	TST_API Int getProgramResourceLocation(UInt program, Enum programInterface, const char *name);

	TST_API Int getProgramResourceLocationIndex(UInt program, Enum programInterface, const char *name);

	TST_API void getProgramResourceName(UInt program, Enum programInterface, UInt index, SizeI bufSize, SizeI *length, char *name);

	TST_API void getProgramStageiv(UInt program, Enum shader_type, Enum pname, Int *values);

	TST_API void getShaderiv(UInt shader, Enum pname, Int *params);

	TST_API void getShaderInfoLog(UInt shader, SizeI maxLength, SizeI *length, Char *infoLog);

	TST_API void getShaderPrecisionFormat(Enum shaderType, Enum precisionType, Int *range, Int *precision);

	TST_API void getShaderSource(UInt shader, SizeI bufSize, SizeI *length, Char *source);

	TST_API UInt getSubroutineIndex(UInt program, Enum shader_type, const Char *name);

	TST_API Int getSubroutineUniformLocation(UInt program, Enum shader_type, const Char *name);

	TST_API void getUniformfv(UInt program, Int location, Float *params);

	TST_API void getUniformiv(UInt program, Int location, Int *params);

	TST_API void getUniformuiv(UInt program, Int location, UInt *params);

	TST_API void getUniformdv(UInt program, Int location, Double *params);

	TST_API void getnUniformfv(UInt program, Int location, SizeI bufSize, Float *params);

	TST_API void getnUniformiv(UInt program, Int location, SizeI bufSize, Int *params);

	TST_API void getnUniformuiv(UInt program, Int location, SizeI bufSize, UInt *params);

	TST_API void getnUniformdv(UInt program, Int location, SizeI bufSize, Double *params);

	TST_API UInt getUniformBlockIndex(UInt program, const Char *uniformBlockName);

	TST_API void getUniformIndices(UInt program, SizeI uniformCount, const Char **uniformNames, UInt *uniformIndices);

	TST_API Int getUniformLocation(UInt program, const Char *name);

	TST_API void getUniformSubroutineuiv(Enum shader_type, Int location, UInt *values);

	TST_API Bool isProgram(UInt program);

	TST_API Bool isShader(UInt shader);

	TST_API void linkProgram(UInt program);

	TST_API void minSampleShading(Float value);

	TST_API void programBinary(UInt program, ShaderBinaryFormat p_binary_format, const void *binary, SizeI length);

	TST_API void programParameteri(UInt program, Enum pname, Int value);

	TST_API void programUniform1f(UInt program, Int location, Float v0);

	TST_API void programUniform2f(UInt program, Int location, Float v0, Float v1);

	TST_API void programUniform3f(UInt program, Int location, Float v0, Float v1, Float v2);

	TST_API void programUniform4f(UInt program, Int location, Float v0, Float v1, Float v2, Float v3);

	TST_API void programUniform1i(UInt program, Int location, Int v0);

	TST_API void programUniform2i(UInt program, Int location, Int v0, Int v1);

	TST_API void programUniform3i(UInt program, Int location, Int v0, Int v1, Int v2);

	TST_API void programUniform4i(UInt program, Int location, Int v0, Int v1, Int v2, Int v3);

	TST_API void programUniform1ui(UInt program, Int location, UInt v0);

	TST_API void programUniform2ui(UInt program, Int location, UInt v0, UInt v1);

	TST_API void programUniform3ui(UInt program, Int location, UInt v0, UInt v1, UInt v2);

	TST_API void programUniform4ui(UInt program, Int location, UInt v0, UInt v1, UInt v2, UInt v3);

	TST_API void programUniform1fv(UInt program, Int location, SizeI count, const Float *p_value);

	TST_API void programUniform2fv(UInt program, Int location, SizeI count, const Float *p_value);

	TST_API void programUniform3fv(UInt program, Int location, SizeI count, const Float *p_value);

	TST_API void programUniform4fv(UInt program, Int location, SizeI count, const Float *p_value);

	TST_API void programUniform1iv(UInt program, Int location, SizeI count, const Int *value);

	TST_API void programUniform2iv(UInt program, Int location, SizeI count, const Int *value);

	TST_API void programUniform3iv(UInt program, Int location, SizeI count, const Int *value);

	TST_API void programUniform4Iv(UInt program, Int location, SizeI count, const Int *value);

	TST_API void programUniform1uiv(UInt program, Int location, SizeI count, const UInt *value);

	TST_API void programUniform2uiv(UInt program, Int location, SizeI count, const UInt *value);

	TST_API void programUniform3uiv(UInt program, Int location, SizeI count, const UInt *value);

	TST_API void programUniform4uiv(UInt program, Int location, SizeI count, const UInt *value);

	TST_API void programUniformMatrix2fv(UInt program, Int location, SizeI count, Bool transpose, const Float *p_value);

	TST_API void programUniformMatrix3fv(UInt program, Int location, SizeI count, Bool transpose, const Float *p_value);

	TST_API void programUniformMatrix4fv(UInt program, Int location, SizeI count, Bool transpose, const Float *p_value);

	TST_API void programUniformMatrix2x3fv(UInt program, Int location, SizeI count, Bool transpose, const Float *p_value);

	TST_API void programUniformMatrix3x2fv(UInt program, Int location, SizeI count, Bool transpose, const Float *p_value);

	TST_API void programUniformMatrix2x4fv(UInt program, Int location, SizeI count, Bool transpose, const Float *p_value);

	TST_API void programUniformMatrix4x2fv(UInt program, Int location, SizeI count, Bool transpose, const Float *p_value);

	TST_API void programUniformMatrix3x4fv(UInt program, Int location, SizeI count, Bool transpose, const Float *p_value);

	TST_API void programUniformMatrix4x3fv(UInt program, Int location, SizeI count, Bool transpose, const Float *p_value);

	TST_API void releaseShaderCompiler();

	TST_API void shaderBinary(SizeI count, const UInt *shaders, ShaderBinaryFormat p_binary_format, const void *binary, SizeI length);

	TST_API void shaderSource(UInt shader, SizeI count, const Char **string, const Int *length);

	TST_API void shaderStorageBlockBinding(UInt program, UInt storageBlockIndex, UInt storageBlockBinding);

	TST_API void specializeShader(UInt        p_shader, const Char *p_entry_point, UInt p_num_specialization_constants, const UInt *p_constant_index,
								  const UInt *p_constant_value);

	TST_API void uniform1f(Int location, Float v0);

	TST_API void uniform2f(Int location, Float v0, Float v1);

	TST_API void uniform3f(Int location, Float v0, Float v1, Float v2);

	TST_API void uniform4f(Int location, Float v0, Float v1, Float v2, Float v3);

	TST_API void uniform1d(Int location, Double v0);

	TST_API void uniform2d(Int location, Double v0, Double v1);

	TST_API void uniform3d(Int location, Double v0, Double v1, Double v2);

	TST_API void uniform4d(Int location, Double v0, Double v1, Double v2, Double v3);

	TST_API void uniform1i(Int location, Int v0);

	TST_API void uniform2i(Int location, Int v0, Int v1);

	TST_API void uniform3i(Int location, Int v0, Int v1, Int v2);

	TST_API void uniform4i(Int location, Int v0, Int v1, Int v2, Int v3);

	TST_API void uniform1ui(Int location, UInt v0);

	TST_API void uniform2ui(Int location, UInt v0, UInt v1);

	TST_API void uniform3ui(Int location, UInt v0, UInt v1, UInt v2);

	TST_API void uniform4ui(Int location, UInt v0, UInt v1, UInt v2, UInt v3);

	TST_API void uniform1fv(Int location, SizeI count, const Float *p_value);

	TST_API void uniform2fv(Int location, SizeI count, const Float *p_value);

	TST_API void uniform3fv(Int location, SizeI count, const Float *p_value);

	TST_API void uniform4fv(Int location, SizeI count, const Float *p_value);

	TST_API void uniform1dv(Int location, SizeI count, const Double *value);

	TST_API void uniform2dv(Int location, SizeI count, const Double *value);

	TST_API void uniform3dv(Int location, SizeI count, const Double *value);

	TST_API void uniform4dv(Int location, SizeI count, const Double *value);

	TST_API void uniform1iv(Int location, SizeI count, const Int *value);

	TST_API void uniform2iv(Int location, SizeI count, const Int *value);

	TST_API void uniform3iv(Int location, SizeI count, const Int *value);

	TST_API void uniform4iv(Int location, SizeI count, const Int *value);

	TST_API void uniform1uiv(Int location, SizeI count, const UInt *value);

	TST_API void uniform2uiv(Int location, SizeI count, const UInt *value);

	TST_API void uniform3uiv(Int location, SizeI count, const UInt *value);

	TST_API void uniform4uiv(Int location, SizeI count, const UInt *value);

	TST_API void uniformMatrix2fv(Int location, SizeI count, Bool transpose, const Float *p_value);

	TST_API void uniformMatrix3fv(Int location, SizeI count, Bool transpose, const Float *p_value);

	TST_API void uniformMatrix4fv(Int location, SizeI count, Bool transpose, const Float *p_value);

	TST_API void uniformMatrix2x3fv(Int location, SizeI count, Bool transpose, const Float *p_value);

	TST_API void uniformMatrix3x2fv(Int location, SizeI count, Bool transpose, const Float *p_value);

	TST_API void uniformMatrix2x4fv(Int location, SizeI count, Bool transpose, const Float *p_value);

	TST_API void uniformMatrix4x2fv(Int location, SizeI count, Bool transpose, const Float *p_value);

	TST_API void uniformMatrix3x4fv(Int location, SizeI count, Bool transpose, const Float *p_value);

	TST_API void uniformMatrix4x3fv(Int location, SizeI count, Bool transpose, const Float *p_value);

	TST_API void uniformBlockBinding(UInt program, UInt uniformBlockIndex, UInt uniformBlockBinding);

	TST_API void uniformSubroutinesuiv(Enum shader_type, SizeI count, const UInt *indices);

	TST_API void useProgram(UInt program);

	TST_API void useProgramStages(UInt pipeline, Bitfield stages, UInt program);

	TST_API void validateProgram(UInt program);

	#pragma endregion

	#pragma region Debug

	TST_API void debugMessageCallback(DebugCallbackProc callback, void *userParam);

	#pragma endregion
}
