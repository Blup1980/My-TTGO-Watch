/****************************************************************************
 *   Sep 3 23:05:42 2020
 *   Copyright  2020  Dirk Brosswick
 *   Email: dirk.brosswick@googlemail.com
 ****************************************************************************/
 
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "config.h"
#include <TTGO.h>

#include "en_app.h"
#include "en_main.h"

#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/app.h"
#include "gui/widget.h"

#include "utils/json_psram_allocator.h"

en_config_t en_config;

// app and widget icon
icon_t *en_app = NULL;
icon_t *en_widget = NULL;

uint32_t en_app_main_tile_num;
uint32_t en_app_setup_tile_num;

// declare you images or fonts you need
LV_IMG_DECLARE(en_64px);

// declare callback functions
static void enter_en_app_event_cb( lv_obj_t * obj, lv_event_t event );

// setup routine for example app
void en_app_setup( void ) {

    // register 2 vertical tiles and get the first tile number and save it for later use
    en_app_main_tile_num = mainbar_add_app_tile( 1, 3, "Enfant App" );
    en_app_setup_tile_num = en_app_main_tile_num + 1;

    en_app = app_register( "Enfant", &en_64px, enter_en_app_event_cb );

    if ( en_config.widget ) {
        en_add_widget();
    }

    en_main_tile_setup( en_app_main_tile_num );
}

uint32_t en_get_app_main_tile_num( void ) {
    return( en_app_main_tile_num );
}

uint32_t en_get_app_setup_tile_num( void ) {
    return( en_app_setup_tile_num );
}

icon_t *en_get_app_icon( void ) {
    return( en_app );
}

icon_t *en_get_widget_icon( void ) {
    return( en_widget );
}

static void enter_en_app_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( en_app_main_tile_num, LV_ANIM_OFF );
                                        statusbar_hide( true );
                                        break;
    }    
}


en_config_t *en_get_config( void ) {
    return( &en_config );
}


bool en_add_widget( void ) {
    if ( en_widget == NULL ) {
        en_widget = widget_register( "n/a", &en_64px, enter_en_app_event_cb );
        widget_hide_indicator( en_widget );
        if ( en_widget != NULL ) {
            return( true );
        }
        else {
            return( false );
        }
    }
    return( true );
}

bool en_remove_widget( void ) {
    en_widget = widget_remove( en_widget );
    return( true );
}
