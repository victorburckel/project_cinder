#include "Resources.h"
#include <boost/range/irange.hpp>
#include <boost/range/algorithm/lower_bound.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>
#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <cinder/gl/gl.h>

namespace io = boost::iostreams;
namespace bpt = boost::property_tree;

namespace {

struct Animation
{
	struct Frame
	{
		uint32_t lastFrame;
		std::array< ci::vec2, 4 > textCoords;
	};

	std::string name;
	std::vector< Frame > frames;
};

Animation loadAnimation( int32_t imageWidth, int32_t imageHeight )
{
	const auto res = ci::app::loadResource( HERO_WALKING_DAT );

	io::basic_array_source< char > content( static_cast< char* >( res->getBuffer()->getData() ), res->getBuffer()->getSize() );
	io::stream< io::basic_array_source< char > > is( content );
	
	bpt::ptree decoded;
	bpt::read_json( is, decoded );

	const auto animation = decoded.get_child( "animation" );

	const auto normalized = [ imageWidth, imageHeight ]( auto x, auto y ) {
		return ci::vec2{ x / imageWidth, ( imageHeight - y ) / imageHeight };
	};

	const auto frames = animation.get_child( "directions" ) | boost::adaptors::transformed( [ normalized ]( const bpt::ptree::value_type& element ) {
		const auto x = element.second.get< float >( "x" );
		const auto y = element.second.get< float >( "y" );
		const auto frameWidth = element.second.get< float >( "frame_width" );
		const auto frameHeight = element.second.get< float >( "frame_height" );

		return Animation::Frame {
			element.second.get< uint32_t >( "num_frames" ),
			{
				normalized( x, y ),
				normalized( x + frameWidth, y ),
				normalized( x, y + frameHeight ),
				normalized( x + frameWidth, y + frameHeight )
		    }
		};
	} );
	
	return Animation{ animation.get< std::string >( "name" ), { frames.begin(), frames.end() } };
}

class CinderApp : public ci::app::App
{
	virtual void setup() override
	{
		const auto image = loadImage( loadResource( HERO_WALKING_IMG ) );
		_animation = loadAnimation( image->getWidth(), image->getHeight() );
		_texture = ci::gl::Texture2d::create( image );

		const auto layouts = std::vector< ci::gl::VboMesh::Layout >{
			ci::gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( ci::geom::Attrib::POSITION, 3 ),
			ci::gl::VboMesh::Layout().usage( GL_DYNAMIC_DRAW ).attrib( ci::geom::Attrib::TEX_COORD_0, 2 )
		};
		const auto mesh = ci::gl::VboMesh::create( ci::geom::Rect( ci::Rectf( -37.5f, -50.f, 37.5f, 50.f ) ), layouts );
		const auto shader = ci::gl::getStockShader( ci::gl::ShaderDef().texture() );
		_batch = ci::gl::Batch::create( mesh, shader );
	}

	virtual void update() override
	{
		const auto& frame = _animation.frames[ ( getElapsedFrames() / 8 ) % 4 ];

		const auto mesh = _batch->getVboMesh();
		auto mappedTextAttrib = _batch->getVboMesh()->mapAttrib2f( ci::geom::Attrib::TEX_COORD_0, false );
		mappedTextAttrib[ 0 ] = frame.textCoords[ 0 ];
		mappedTextAttrib[ 1 ] = frame.textCoords[ 1 ];
		mappedTextAttrib[ 2 ] = frame.textCoords[ 2 ];
		mappedTextAttrib[ 3 ] = frame.textCoords[ 3 ];
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
	Animation _animation;
};

}

CINDER_APP( CinderApp, ci::app::RendererGl, []( ci::app::App::Settings* settings ) { settings->setResizable( false ); settings->setFrameRate( 60. ); } )