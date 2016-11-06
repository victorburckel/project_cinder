#include "Resources.h"
#include <boost/range/irange.hpp>
#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <cinder/gl/gl.h>

namespace {

class CinderApp : public ci::app::App
{
	virtual void setup() override
	{
		const auto image = loadImage( loadResource( HERO_WALKING ) );
		_texture = ci::gl::Texture2d::create( image );

		const auto layouts = std::vector< ci::gl::VboMesh::Layout >{
			ci::gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( ci::geom::Attrib::POSITION, 3 ),
			ci::gl::VboMesh::Layout().usage( GL_DYNAMIC_DRAW ).attrib( ci::geom::Attrib::TEX_COORD_0, 2 )
		};
		const auto mesh = ci::gl::VboMesh::create( ci::geom::Rect( ci::Rectf( -50.f, -50.f, 50, 50.f ) ), layouts );
		const auto shader = ci::gl::getStockShader( ci::gl::ShaderDef().texture() );
		_batch = ci::gl::Batch::create( mesh, shader );
	}

	virtual void update() override
	{
		const auto mesh = _batch->getVboMesh();
		auto mappedTextAttrib = _batch->getVboMesh()->mapAttrib2f( ci::geom::Attrib::TEX_COORD_0, false );
		mappedTextAttrib[ 0 ] = ci::vec2( 0.f, 1.f );
		mappedTextAttrib[ 1 ] = ci::vec2( 0.2, 1.f );
		mappedTextAttrib[ 2 ] = ci::vec2( 0.f, 0.8f );
		mappedTextAttrib[ 3 ] = ci::vec2( 0.2f, 0.8f );
		mappedTextAttrib.unmap();
	}

	virtual void draw() override
	{
		ci::gl::clear( ci::Color( 0.2f, 0.3f, 0.3f ) );
		ci::gl::ScopedTextureBind texture( _texture );
		ci::gl::ScopedModelMatrix scpMtx;
		ci::gl::translate( getWindowCenter() );
		_batch->draw();
	}

	ci::gl::Texture2dRef _texture;
	ci::gl::BatchRef _batch;
	ci::gl::VboMeshRef _mesh;
};

}

CINDER_APP( CinderApp, ci::app::RendererGl, []( ci::app::App::Settings* settings ) { settings->setResizable( false ); } )