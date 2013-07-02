/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013 Andrew Mickelson
 *
 *  This file is part of Attract-Mode.
 *
 *  Attract-Mode is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Attract-Mode is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Attract-Mode.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "fe_input.hpp"
#include "fe_util.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/System/Vector2.hpp>

FeMouseCapture::FeMouseCapture( int aradius )
	: capture_count( 0 )
{
	sf::VideoMode vm = sf::VideoMode::getDesktopMode();

	reset_x = vm.width / 2;
	reset_y = vm.height / 2;
	left_bound = reset_x - aradius;
	right_bound = reset_x + aradius;
	top_bound = reset_y - aradius;
	bottom_bound = reset_y + aradius;
}

FeMapping::FeMapping( FeInputMap::Command cmd )
	: command( cmd )
{
}

void FeMapping::add_input( const std::pair<int,FeInputMap::InputType> &index )
{
   if ( index.second == FeInputMap::Keyboard )
      input_list.push_back( FeInputMap::keyTable[ index.first ].label );
   else
      input_list.push_back( FeInputMap::inputStrings[ index.second ] );
}

bool FeMapping::operator< ( const FeMapping o ) const
{
	return ( command < o.command );
}

FeInputMap::FeInputMap( bool disable_mousecap )
	: m_cap( 30 ), 
	m_disable_mousecap( disable_mousecap )
{
}

void FeInputMap::default_mappings()
{
	//
	// Only set default mappings if there has been no mapping by user
	//
	if ( !m_map.empty() )
		return;

	//
	// Set up default input mappings. 
	//
	struct DefaultMappings { int k; InputType i; Command c; };
	DefaultMappings dmap[] =
	{
		{ sf::Keyboard::Escape, Keyboard, ExitMenu },
		{ sf::Keyboard::Up, Keyboard, Up },
		{ sf::Keyboard::Down, Keyboard, Down },
		{ sf::Keyboard::Left, Keyboard, PrevList },
		{ sf::Keyboard::Right, Keyboard, NextList },
		{ sf::Keyboard::Return, Keyboard, Select },
		{ sf::Keyboard::Tab, Keyboard, Configure },
		{ sf::Keyboard::Unknown, LAST_INPUT, LAST_COMMAND }	// keep as last
	};

	int i=0;
	while ( dmap[i].i != LAST_INPUT )
	{
		m_map[ std::pair<int,InputType>( dmap[i].k, dmap[i].i ) ] = dmap[i].c;
		i++;
	}
}

// NOTE: This needs to be kept in alignment with enum FeInputMap::Command
//
const char *FeInputMap::commandStrings[] =
{
	"select",
	"up",
	"down",
	"page_up",
	"page_down",
	"prev_list",
	"next_list",
	"toggle_layout",
	"toggle_movie",
	"toggle_mute",
	"toggle_rotate_right",
	"toggle_flip",
	"toggle_rotate_left",
	"exit",
	"exit_no_menu",
	"screenshot",
	"configure",
	NULL, // LAST_COMMAND... NULL required here
	"ambient",
	"startup",
	"game_return",
	NULL
};
const char *FeInputMap::commandDispStrings[] =
{
	"Select",
	"Up",
	"Down",
	"Page Up",
	"Page Down",
	"Previous List",
	"Next List",
	"Toggle Layout",
	"Toggle Movie",
	"Toggle Mute",
	"Toggle Rotate Right",
	"Toggle Flip",
	"Toggle Rotate Left",
	"Exit (Confirm)",
	"Exit to Desktop",
	"Screenshot",
	"Configure",
	NULL, // LAST_COMMAND... NULL required here
	"Ambient Sounds",
	"Startup Sound",
	"Game Return Sound",
	NULL
};

std::pair< int, FeInputMap::InputType > 
FeInputMap::get_map_index( sf::Event e, bool config )
{
	std::pair< int, InputType > index( 0, LAST_INPUT );

	switch ( e.type )
	{

		case sf::Event::KeyPressed:
			index.first = e.key.code;
			index.second = Keyboard;
			break;

		case sf::Event::JoystickButtonPressed:
			index.first = e.joystickButton.joystickId + 1;
			switch ( e.joystickButton.button )
			{
				case 0: index.second=JoyB1; break;
				case 1: index.second=JoyB2; break;
				case 2: index.second=JoyB3; break;
				case 3: index.second=JoyB4; break;
				case 4: index.second=JoyB5; break;
				case 5: index.second=JoyB6; break;
				case 6: index.second=JoyB7; break;
				case 7: index.second=JoyB8; break;
			}
			break;

		case sf::Event::JoystickMoved:
			index.first = e.joystickMove.joystickId + 1;

			if ( e.joystickMove.axis == sf::Joystick::X )
			{
				if ( abs( e.joystickMove.position ) > JOY_THRESH )
				{
					if ( e.joystickMove.position > 0 )
						index.second = JoyRight;
					else 
						index.second = JoyLeft;
				}
			}
			else if ( e.joystickMove.axis == sf::Joystick::Y )
			{
				if ( abs( e.joystickMove.position ) > JOY_THRESH )
				{
					if ( e.joystickMove.position > 0 )
						index.second = JoyDown;
					else 
						index.second = JoyUp;
				}
			}
			break;
		
		case sf::Event::MouseMoved:
			if (( m_cap.capture_count || config ) && ( !m_disable_mousecap ))
			{
				sf::Vector2i mpos = sf::Mouse::getPosition();
				bool reset_mouse=false;
				if ( mpos.x < m_cap.left_bound )
				{
					index.second=MouseLeft;
					reset_mouse = true;
				}
				else if ( mpos.x > m_cap.right_bound )
				{
					index.second=MouseRight;
					reset_mouse = true;
				}
				else if ( mpos.y < m_cap.top_bound )
				{
					index.second=MouseUp;
					reset_mouse = true;
				}
				else if ( mpos.y > m_cap.bottom_bound )
				{
					index.second=MouseDown;
					reset_mouse = true;
				}

				if ( reset_mouse && !config )
				{
					sf::Mouse::setPosition( 
							sf::Vector2i( m_cap.reset_x, m_cap.reset_y ) );
				}
			}
			break;

		case sf::Event::MouseWheelMoved:
			if ( e.mouseWheel.delta > 0 )
				index.second=MouseWheelUp;
			else 
				index.second=MouseWheelDown;
			break;

		case sf::Event::MouseButtonPressed:
			switch ( e.mouseButton.button )
			{
				case sf::Mouse::Left: index.second=MouseBLeft; break;
				case sf::Mouse::Right: index.second=MouseBRight; break;
				case sf::Mouse::Middle: index.second=MouseBMiddle; break;
				case sf::Mouse::XButton1: index.second=MouseBX1; break;
				case sf::Mouse::XButton2: index.second=MouseBX2; break;
				default: break; 
			}
			break;

		default:
			break;
	}

	return index;
}

FeInputMap::Command FeInputMap::map( sf::Event e )
{
	if ( e.type == sf::Event::Closed )
		return ExitNoMenu;

	std::pair< int, InputType > index = get_map_index( e, false );
	if ( index.second == LAST_INPUT )
		return LAST_COMMAND;

	std::map< std::pair< int, InputType>,Command>::iterator it;
	it = m_map.find( index );

	if ( it == m_map.end() )
	{
		if (( e.type == sf::Event::JoystickMoved )
				|| ( e.type == sf::Event::JoystickButtonPressed ))
		{
			// check if mapped for all joysticks ( index.first=0 )
			index.first=0;
			it = m_map.find( index );
			if ( it == m_map.end() )
				return LAST_COMMAND;
		}
		else
			return LAST_COMMAND;
	}

	return (*it).second;
}

void FeInputMap::get_mappings( std::vector< FeMapping > &mappings )
{
	//
	// Make a mappings entry for each possible command mapping
	//
	mappings.clear();
	mappings.reserve( LAST_COMMAND );
	for ( int i=0; i< LAST_COMMAND; i++ )
		mappings.push_back( FeMapping( (FeInputMap::Command)i ) );

	//
	// Now populate the mappings vector with the various input mappings
	//
	std::map< std::pair< int, InputType>,Command>::iterator it;

	for ( it=m_map.begin(); it!=m_map.end(); ++it )
		mappings[ (*it).second ].add_input( (*it).first );
}

void FeInputMap::set_mapping( const FeMapping &mapping )
{
	Command cmd = mapping.command;

	if ( cmd == LAST_COMMAND )
		return;

	//
	// Erase existing mappings to this command
	//
	std::map< std::pair< int, InputType>,Command>::iterator it = m_map.begin();
	while ( it != m_map.end() )
	{
		if ( (*it).second == cmd )
		{
			std::map< std::pair< int, InputType>,Command>::iterator to_erase = it;
			++it;

			// If this was a mouse move then decrement our capture count
			//
			if (is_mouse_move( (*to_erase).first.second ))
			{
				m_cap.capture_count--;
				ASSERT( m_cap.capture_count >= 0 );
			}
			m_map.erase( to_erase );
		}
		else
			++it;
	}
	
	//
	// Now update our map with the inputs from this mapping 
	//
	std::vector< std::string >::const_iterator iti;

	for ( iti=mapping.input_list.begin(); 
			iti!=mapping.input_list.end(); ++iti )
	{
		std::pair< int, InputType > index( 0, LAST_INPUT );
		string_to_index( (*iti), index );

		if (index.second != LAST_INPUT )
		{
			m_map[ index ] = cmd;

			// Turn on mouse capture if we are interested in mouse movements
			//
			if (is_mouse_move( index.second ))
				m_cap.capture_count++;
		}
	}
}

void FeInputMap::init_config_map_input()
{
	sf::Mouse::setPosition( sf::Vector2i( m_cap.reset_x, m_cap.reset_y ) );
}

bool FeInputMap::config_map_input( sf::Event e, std::string &s, Command &conflict )
{
	conflict = LAST_COMMAND;
	std::pair< int, InputType > index = get_map_index( e, true );
	if ( index.second == LAST_INPUT )
		return false;

   if ( index.second == FeInputMap::Keyboard )
      s = keyTable[ index.first ].label;
   else
      s = inputStrings[ index.second ];

	//
	// Now find if there is a conflicting existing mapping
	//
	std::map< std::pair< int, InputType>,Command>::iterator it;
	it = m_map.find( index );

	if ( it == m_map.end() )
	{
		if (( e.type == sf::Event::JoystickMoved )
				|| ( e.type == sf::Event::JoystickButtonPressed ))
		{
			// check if mapped for all joysticks ( index.first=0 )
			//
			index.first=0;
			it = m_map.find( index );
			if ( it != m_map.end() )
				conflict = (*it).second;
		}
	}
	else
		conflict = (*it).second;
	
	return true;
}

const FeInputMap::KeyLookup FeInputMap::keyTable[] = 
{
	{ "A", sf::Keyboard::A },
	{ "B", sf::Keyboard::B },
	{ "C", sf::Keyboard::C },
	{ "D", sf::Keyboard::D },
	{ "E", sf::Keyboard::E },
	{ "F", sf::Keyboard::F },
	{ "G", sf::Keyboard::G },
	{ "H", sf::Keyboard::H },
	{ "I", sf::Keyboard::I },
	{ "J", sf::Keyboard::J },
	{ "K", sf::Keyboard::K },
	{ "L", sf::Keyboard::L },
	{ "M", sf::Keyboard::M },
	{ "N", sf::Keyboard::N },
	{ "O", sf::Keyboard::O },
	{ "P", sf::Keyboard::P },
	{ "Q", sf::Keyboard::Q },
	{ "R", sf::Keyboard::R },
	{ "S", sf::Keyboard::S },
	{ "T", sf::Keyboard::T },
	{ "U", sf::Keyboard::U },
	{ "V", sf::Keyboard::V },
	{ "W", sf::Keyboard::W },
	{ "X", sf::Keyboard::X },
	{ "Y", sf::Keyboard::Y },
	{ "Z", sf::Keyboard::Z },
	{ "0", sf::Keyboard::Num0 },
  	{ "1", sf::Keyboard::Num1 },
	{ "2", sf::Keyboard::Num2 },
	{ "3", sf::Keyboard::Num3 },
	{ "4", sf::Keyboard::Num4 },
	{ "5", sf::Keyboard::Num5 },
	{ "6", sf::Keyboard::Num6 },
	{ "7", sf::Keyboard::Num7 },
	{ "8", sf::Keyboard::Num8 },
	{ "9", sf::Keyboard::Num9 },
	{ "Escape", sf::Keyboard::Escape },
	{ "LControl", sf::Keyboard::LControl },
	{ "LShift", sf::Keyboard::LShift },
	{ "LAlt", sf::Keyboard::LAlt },
	{ "LSystem", sf::Keyboard::LSystem },
	{ "RControl", sf::Keyboard::RControl },
	{ "RShift", sf::Keyboard::RShift },
	{ "RAlt", sf::Keyboard::RAlt },
	{ "RSystem", sf::Keyboard::RSystem },
	{ "Menu", sf::Keyboard::Menu },
	{ "[", sf::Keyboard::LBracket },
	{ "]", sf::Keyboard::RBracket },
	{ ";", sf::Keyboard::SemiColon },
	{ ",", sf::Keyboard::Comma },
	{ ".", sf::Keyboard::Period },
	{ "'", sf::Keyboard::Quote },
	{ "/", sf::Keyboard::Slash },
	{ "\\", sf::Keyboard::BackSlash },
	{ "~", sf::Keyboard::Tilde },
	{ "=", sf::Keyboard::Equal },
	{ "-", sf::Keyboard::Dash },
	{ "Space", sf::Keyboard::Space },
	{ "Return", sf::Keyboard::Return },
	{ "Backspace", sf::Keyboard::BackSpace },
	{ "Tab", sf::Keyboard::Tab },
	{ "PageUp", sf::Keyboard::PageUp },
	{ "PageDown", sf::Keyboard::PageDown },
	{ "End", sf::Keyboard::End },
	{ "Home", sf::Keyboard::Home },
	{ "Insert", sf::Keyboard::Insert },
	{ "Delete", sf::Keyboard::Delete },
	{ "Add", sf::Keyboard::Add },
	{ "Subtract", sf::Keyboard::Subtract },
	{ "Multiply", sf::Keyboard::Multiply },
	{ "Divide", sf::Keyboard::Divide },
	{ "Left", sf::Keyboard::Left },
	{ "Right", sf::Keyboard::Right },
	{ "Up", sf::Keyboard::Up },
	{ "Down", sf::Keyboard::Down },
	{ "Numpad0", sf::Keyboard::Numpad0 },
	{ "Numpad1", sf::Keyboard::Numpad1 },
	{ "Numpad2", sf::Keyboard::Numpad2 },
	{ "Numpad3", sf::Keyboard::Numpad3 },
	{ "Numpad4", sf::Keyboard::Numpad4 },
	{ "Numpad5", sf::Keyboard::Numpad5 },
	{ "Numpad6", sf::Keyboard::Numpad6 },
	{ "Numpad7", sf::Keyboard::Numpad7 },
	{ "Numpad8", sf::Keyboard::Numpad8 },
	{ "Numpad9", sf::Keyboard::Numpad9 },
	{ "F1", sf::Keyboard::F1 },
	{ "F2", sf::Keyboard::F2 },
	{ "F3", sf::Keyboard::F3 },
	{ "F4", sf::Keyboard::F4 },
	{ "F5", sf::Keyboard::F5 },
	{ "F6", sf::Keyboard::F6 },
	{ "F7", sf::Keyboard::F7 },
	{ "F8", sf::Keyboard::F8 },
	{ "F9", sf::Keyboard::F9 },
	{ "F10", sf::Keyboard::F10 },
	{ "F11", sf::Keyboard::F11 },
	{ "F12", sf::Keyboard::F12 },
	{ "F13", sf::Keyboard::F13 },
	{ "F14", sf::Keyboard::F14 },
	{ "F15", sf::Keyboard::F15 },
	{ "Pause", sf::Keyboard::Pause },
	{ NULL, sf::Keyboard::Unknown } // needs to be last
};

const char *FeInputMap::inputStrings[] = 
// needs to stay aligned with InputType enum
{	
	"JoystickUp",
	"JoystickDown",
	"JoystickLeft",
	"JoystickRight",
	"JoystickButton1",
	"JoystickButton2",
	"JoystickButton3",
	"JoystickButton4",
	"JoystickButton5",
	"JoystickButton6",
	"JoystickButton7",
	"JoystickButton8",
	"MouseUp",
	"MouseDown",
	"MouseLeft",
	"MouseRight",
	"MouseWheelUp",
	"MouseWheelDown",
	"MouseLeftButton",
	"MouseRightButton",
	"MouseMiddleButton",
	"MouseExtraButton1",
	"MouseExtraButton2",
	// Keyboard doesn't get an entry 
	NULL
};

int FeInputMap::process_setting( const std::string &setting, 
								const std::string &value,
								const std::string &fn )
{

	Command cmd = string_to_command( setting );
	if ( cmd == LAST_COMMAND )
	{
		invalid_setting( fn, "input_map", setting, commandStrings, NULL, "command" );
		return 1;
	}

	std::pair< int, InputType > index( 0, LAST_INPUT );
	string_to_index( value, index );

	if ( index.second == LAST_INPUT )
	{
		const char *valid[ sf::Keyboard::KeyCount + 1 ];
		for ( int i=0; i< sf::Keyboard::KeyCount; i++ )
			valid[i] = keyTable[i].label;

		valid[ sf::Keyboard::KeyCount ] = '\0';
		
		invalid_setting( fn, "input_map", value, valid, inputStrings, "key" );
		return 1;
	}

	m_map[index] = cmd;

	// Turn on mouse capture if we are interested in mouse movements
	if ( is_mouse_move( index.second ) )
		m_cap.capture_count++;

	return 0;
}

void FeInputMap::save( std::ofstream &f )
{
	std::map< std::pair < int, InputType >, Command >::iterator it;

	for ( it = m_map.begin(); it != m_map.end(); ++it )
	{
   	f << '\t' << std::setw(20) << std::left 
			<< commandStrings[ (*it).second ] << ' ';

		const std::pair< int, InputType > &index( (*it).first );

   	if ( index.second == FeInputMap::Keyboard )
      	f << keyTable[ index.first ].label;
   	else
     		f << inputStrings[ index.second ];

		if ( is_joystick( index.second  ) 
				&& ( index.first > 0 )
				&& ( index.first <= sf::Joystick::Count ))
     		f << ' ' << index.first;

		f << std::endl;
	}
}

bool FeInputMap::is_mouse_move( InputType i )
{
	return (( i == MouseUp ) || ( i == MouseDown ) 
			|| ( i == MouseLeft ) || ( i == MouseRight ));
}


FeInputMap::Command FeInputMap::string_to_command( const std::string &s )
{
   Command cmd( LAST_COMMAND );
   int i=0;

   while ( FeInputMap::commandStrings[i] != NULL )
   {
      if ( s.compare( commandStrings[i] ) == 0 )
      {
         cmd = (Command)i;
         break;
      }
      i++;
   }
   return cmd;
}

void FeInputMap::string_to_index( const std::string &s,
				std::pair< int, InputType > &index )
{
	index.first = 0;
	index.second = LAST_INPUT;

	size_t pos=0;
	std::string val;

	token_helper( s, pos, val, FE_WHITESPACE );
	bool match=false;
	int i=0;

	while ( inputStrings[i] != NULL )
	{
		if ( val.compare( inputStrings[i] ) == 0 )
		{
			index.second = (InputType)i;
			match=true;
			break;
		}
		i++;
	}
	i=0;
	
	if (!match)
	{
		while ( keyTable[i].label != NULL )
		{
			if ( val.compare( keyTable[i].label ) == 0 )
			{
				index.first = keyTable[i].key;
				index.second = Keyboard;
				match=true;
				break;
			}
			i++;
		}
	}

	if (!match)
		return;

	// There can be a third entry on an input map line in the case of 
	// joystick input.  this entry can specify which joystick the mapping
	// belongs to.  In the absense of this third entry, the mapping is to
	// all joysticks
	// 
	if ( is_joystick( index.second ) && ( pos < s.size() ))
	{
		std::string val;
		token_helper( s, pos, val );
		if ( !val.empty() )
		{
			int joyid = as_int( val );

			if (( joyid > 0 ) && ( joyid <= sf::Joystick::Count ))
				index.first = joyid;
			else
			{
				std::cout << "Invalid Joystick number: " << joyid 
					<< ", valid: 1 to " << sf::Joystick::Count << std::endl;
			}
		}
	}
}

// Note the alignment of settingStrings matters in fe_config.cpp
// (FeSoundMenu::get_options)
//
const char *FeSoundInfo::settingStrings[] =
{
	"sound_volume",
	"ambient_volume",
	"movie_volume",
	NULL
};
const char *FeSoundInfo::settingDispStrings[] =
{
	"Sound Volume",
	"Ambient Volume",
	"Movie Volume",
	NULL
};

FeSoundInfo::FeSoundInfo()
	: m_sound_vol( 100 ), 
	m_ambient_vol( 100 ), 
	m_movie_vol( 100 ),
	m_mute( false )
{
}

void FeSoundInfo::set_volume( SoundType t, const std::string &str )
{
	int v = as_int( str );

	if ( v < 0 )
		v = 0;
	else if ( v > 100 )
		v = 100;

	switch ( t )
	{
	case Movie:
		m_movie_vol = v;
		break;

	case Ambient:
		m_ambient_vol = v;
		break;

	case Sound:
	default:
		m_sound_vol = v;
	}
}

int FeSoundInfo::get_set_volume( SoundType t )
{
	switch ( t )
	{
	case Movie:
		return m_movie_vol;
	case Ambient:
		return m_ambient_vol;
	case Sound:
	default:
		return m_sound_vol;
	}
}

int FeSoundInfo::get_play_volume( SoundType t )
{
	if ( m_mute )
		return 0;

	return get_set_volume( t );
}

bool FeSoundInfo::get_mute()
{
	return m_mute;
}

void FeSoundInfo::set_mute( bool m )
{
	m_mute=m;
}

int FeSoundInfo::process_setting( const std::string &setting,
							const std::string &value,
							const std::string &fn )
{
	if ( setting.compare( settingStrings[0] ) == 0 ) // sound_vol
		set_volume( Sound, value );
	else if ( setting.compare( settingStrings[1] ) == 0 ) // ambient_vol
		set_volume( Ambient, value );
	else if ( setting.compare( settingStrings[2] ) == 0 ) // movie_vol
		set_volume( Movie, value );
	else 
	{
		bool found=false;
		for ( int i=0; i < FeInputMap::LAST_EVENT; i++ )
		{
			if ( ( FeInputMap::commandStrings[i] ) 
				&& ( setting.compare( FeInputMap::commandStrings[i] ) == 0 ))
			{
				found=true;
				m_sounds[ (FeInputMap::Command)i ] = value;
				break;
			}
		}

		if (!found)
		{
			invalid_setting( fn, "sound", setting, settingStrings, FeInputMap::commandStrings );
			return 1;
		}
	}

	return 0;
}

bool FeSoundInfo::get_sound( FeInputMap::Command c, std::string &name )
{
	if (( !m_mute ) && ( m_sound_vol > 0 ))
	{
		std::map<FeInputMap::Command, std::string>::iterator it;

   	it = m_sounds.find( c );

   	if ( it == m_sounds.end() )
     		return false;

		name = (*it).second;
	}
	return true;
}

void FeSoundInfo::set_sound( FeInputMap::Command c, const std::string &name )
{
	if ( name.empty() )
		m_sounds.erase( c );
	else
		m_sounds[ c ] = name;
}

void FeSoundInfo::save( std::ofstream &f )
{
	std::map<FeInputMap::Command, std::string>::iterator it;

	for ( int i=0; i< 3; i++ )
   	f << '\t' << std::setw(20) << std::left << settingStrings[i] << ' ' 
			<< get_set_volume( (SoundType)i ) << std::endl;

	for ( it=m_sounds.begin(); it!=m_sounds.end(); ++it )
   	f << '\t' << std::setw(20) << std::left 
			<< FeInputMap::commandStrings[ (*it).first ] 
			<< ' ' << (*it).second << std::endl;
}
