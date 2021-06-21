#include "..\cmenu.hpp"

C_Button::C_Button( std::string n_label, std::function< void( ) > n_func ) {
	parent = g_cfg.menu.last_group;
	label = n_label;
	func = n_func;

	area.h = 20;

	parent->add_control( this );
}

void C_Button::draw( ) {
	render::get( ).rect_filled( area.x, area.y, area.w, 20, Color { 24, 24, 24 } );

	POINT mouse; GetCursorPos( &mouse );
	rect_t item_region = rect_t( area.x, area.y, area.w, 20 );

	if ( item_region.contains_point( mouse ) ) {
		render::get( ).rect_filled( area.x, area.y, area.w, 20, Color { 30, 30, 30 } );
	}

	render::get().rect(area.x, area.y, area.w, 20, Color{49,49,49});

	render::get( ).text( fonts[ TAHOMA12 ], area.x + 10, area.y + 10, Color::White, HFONT_CENTERED_Y, label.c_str( ) );
}

void C_Button::update( ) {

}

void C_Button::click( ) {
	func( );
}