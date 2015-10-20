#include "Window.h"

#include <future>
#include <thread>
#include <SFML/Graphics.hpp>

#if defined(_DEBUG)
#pragma comment(lib, "sfml-graphics-s-d.lib")
#pragma comment(lib, "sfml-window-s-d.lib")
#pragma comment(lib, "sfml-system-s-d.lib")
#else
#pragma comment(lib, "sfml-graphics-s.lib")
#pragma comment(lib, "sfml-window-s.lib")
#pragma comment(lib, "sfml-system-s.lib")
#endif

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "freetype.lib")
#pragma comment(lib, "jpeg.lib")

class WindowImpl
{
	std::future<void> m_window_proc;
	bool m_closing = false;
	float m_scale = 1.0f;

	std::mutex m_mutex;
	std::vector<std::unique_ptr<sf::Shape>> m_shapes;

public:
	WindowImpl( std::string title, int width, int height )
	{
		const int max_width = 1800;
		const int max_height = 900;

		if( width > max_width )
			m_scale = std::min( m_scale, float( max_width ) / float( width ) );

		if( height > max_height )
			m_scale = std::min( m_scale, float( max_height ) / float( height ) );

		width = int( float( width ) * m_scale );
		height = int( float( height ) * m_scale );

		m_window_proc = std::async( std::launch::async, [this, title, width, height]
		{
			sf::RenderWindow sfml_window( sf::VideoMode( width, height ), title );
			float current_zoom = 1.0f;
			bool is_dragging = false;
			int mouse_drag_x = 0;
			int mouse_drag_y = 0;

			while( sfml_window.isOpen() && !m_closing )
			{
				sf::Event event;
				while( sfml_window.pollEvent( event ) )
				{
					switch( event.type )
					{
					case sf::Event::Closed:
					{
						sfml_window.close();
						break;
					}

					case sf::Event::MouseWheelScrolled:
					{
						auto view = sfml_window.getView();
						float zoom_factor = event.mouseWheelScroll.delta > 0.0f ? 1.5f : 0.6666666f;
						view.zoom( zoom_factor );
						current_zoom *= zoom_factor;
						sfml_window.setView( view );
						
						break;
					}

					case sf::Event::MouseButtonPressed:
					{
						if( event.mouseButton.button == sf::Mouse::Button::Left )
						{
							is_dragging = true;
							mouse_drag_x = event.mouseButton.x;
							mouse_drag_y = event.mouseButton.y;
						}

						break;
					}

					case sf::Event::MouseButtonReleased:
					{
						if( event.mouseButton.button == sf::Mouse::Button::Left )
							is_dragging = false;

						break;
					}

					case sf::Event::MouseMoved:
					{
						if( !is_dragging )
							break;

						float movement_x = float( event.mouseMove.x - mouse_drag_x ) * current_zoom;
						float movement_y = float( event.mouseMove.y - mouse_drag_y ) * current_zoom;
						mouse_drag_x = event.mouseMove.x;
						mouse_drag_y = event.mouseMove.y;

						auto view = sfml_window.getView();
						view.move( -movement_x, -movement_y );
						sfml_window.setView( view );

						break;
					}
					}
				}

				{
					sfml_window.clear();

					std::lock_guard<std::mutex> lock( m_mutex );
					for( auto & shape : m_shapes )
						sfml_window.draw( *shape );
				}

				sfml_window.display();

				std::this_thread::sleep_for( std::chrono::milliseconds{ 16 } );
			}
		} );
	}

	~WindowImpl()
	{
		m_closing = true;
		if( m_window_proc.valid() )
			m_window_proc.wait();
	}

	void addBox( int left, int top, int right, int bottom, Color color )
	{
		std::lock_guard<std::mutex> lock( m_mutex );

		auto new_shape = std::make_unique<sf::RectangleShape>( sf::Vector2f( float( right - left ), float( bottom - top ) ) * m_scale );
		new_shape->setPosition( sf::Vector2f( float( left ), float( top ) ) * m_scale );
		new_shape->setFillColor( sf::Color( color.red(), color.green(), color.blue() ) );
		m_shapes.push_back( std::move( new_shape ) );
	}
};

Window::Window() = default;

Window::Window( std::string title, int width, int height )
{
	m_impl = std::make_unique<WindowImpl>( std::move( title ), width, height );
}

Window::~Window() = default;

void Window::addBox( int left, int top, int right, int bottom, Color color )
{
	if( m_impl )
		m_impl->addBox( left, top, right, bottom, color );
}

void Window::addBox( Position top_left, Position bottom_right, Color color )
{
	addBox( top_left.x, top_left.y, bottom_right.x, bottom_right.y, color );
}