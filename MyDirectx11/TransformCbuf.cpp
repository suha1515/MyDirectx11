#include "TransformCbuf.h"

TransformCbuf::TransformCbuf(Graphics& gfx, const Drawable& parent)
	:vcbuf(gfx),parent(parent)
{}

void TransformCbuf::Bind(Graphics& gfx) noexcept
{
}
