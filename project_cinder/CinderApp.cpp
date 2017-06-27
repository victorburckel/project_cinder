#include "Resources.h"
#include <boost/range/irange.hpp>
#include <boost/range/algorithm/lower_bound.hpp>
#include <boost/range/algorithm/generate.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>
#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <cinder/params/Params.h>
#include <cinder/gl/gl.h>

namespace io = boost::iostreams;
namespace bpt = boost::property_tree;

namespace{

struct InstanceData
{
	ci::vec2 position;
	ci::vec4 textCoords;
};

struct Animation
{
	struct Frame
	{
		uint32_t lastFrame;
		float x;
		float y;
		float frame_width;
		float frame_height;
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

	const auto frames = animation.get_child( "directions" ) | boost::adaptors::transformed( [ imageWidth, imageHeight ]( const auto& element ){
		return Animation::Frame{
			element.second.get< uint32_t >( "num_frames" ),
			element.second.get< float >( "x" ) / imageWidth,
			1 - ( element.second.get< float >( "y" ) + element.second.get< float >( "frame_height" ) ) / imageHeight, 
			element.second.get< float >( "frame_width" ) / imageWidth,
			element.second.get< float >( "frame_height" ) / imageHeight
		};
	} );

	return Animation{ animation.get< std::string >( "name" ), { frames.begin(), frames.end() } };
}

class CinderApp : public ci::app::App
{
	virtual void setup() override
	{
		try
		{
			_params = { "Parameters", ci::vec2( 200, 50 ) };
			_params.addParam( "Sprites", &_instances ).min( 1 ).max( maxInstances );
			_params.addParam( "Fps", &_averageFps, true );

			const auto image = loadImage( loadResource( HERO_WALKING_IMG ) );
			_animation = loadAnimation( image->getWidth(), image->getHeight() );
			_texture = ci::gl::Texture2d::create( image );

			std::vector< InstanceData > instances( maxInstances );
			_instancesData = ci::gl::Vbo::create( GL_ARRAY_BUFFER, instances, GL_DYNAMIC_DRAW );

			ci::geom::BufferLayout instanceDataLayout;

			instanceDataLayout.append( ci::geom::Attrib::CUSTOM_0, 2, sizeof( InstanceData ), offsetof( InstanceData, position ), 1 );
			instanceDataLayout.append( ci::geom::Attrib::CUSTOM_1, 4, sizeof( InstanceData ), offsetof( InstanceData, textCoords ), 1 );

			const auto mesh = ci::gl::VboMesh::create( ci::geom::Rect( ci::Rectf( -37.5f, -50.f, 37.5f, 50.f ) ) );
			mesh->appendVbo( instanceDataLayout, _instancesData );

			ci::gl::Batch::AttributeMapping mapping;
			mapping[ ci::geom::Attrib::CUSTOM_0 ] = "aInstancePosition";
			mapping[ ci::geom::Attrib::CUSTOM_1 ] = "aInstanceTextCoords";

			const auto shader = ci::gl::GlslProg::create( loadResource( VERTEX_SHADER ), loadResource( FRAMGENT_SHADER ) );
			_batch = ci::gl::Batch::create( mesh, shader, mapping );
		}
		catch( const std::exception &e )
		{
			console() << e.what() << std::endl;
			quit();
		}
	}

	virtual void update() override
	{
		const auto& frame = _animation.frames[ ( getElapsedFrames() / 8 ) % 4 ];

		const auto instancesData = static_cast< InstanceData* >( _instancesData->mapReplace() );
		for( auto& instanceData : boost::make_iterator_range( instancesData, instancesData + _instances ) )
			instanceData.textCoords = { frame.x, frame.y, frame.frame_width, frame.frame_height };
		_instancesData->unmap();
		_averageFps = getAverageFps();
	}

	virtual void draw() override
	{
		ci::gl::clear( ci::Color( 0.2f, 0.3f, 0.3f ) );
		
		{
			ci::gl::ScopedTextureBind texture( _texture );
			ci::gl::ScopedModelMatrix scpMtx;
			ci::gl::translate( getWindowCenter() ),
			_batch->drawInstanced( _instances );
		}

		_params.draw();
	}

	ci::gl::Texture2dRef _texture;
	ci::gl::VboRef _instancesData;
	ci::gl::BatchRef _batch;
	Animation _animation;
	ci::params::InterfaceGl _params;
	int _instances = 1000;
	float _averageFps = 0.;
	const int maxInstances = 10000;
};

}

CINDER_APP( CinderApp, ci::app::RendererGl, []( ci::app::App::Settings* settings ){ settings->setResizable( true ); settings->disableFrameRate(); } )