/****************************************************************************
 *   Aug 3 12:17:11 2020
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
#include <WiFi.h>
#include <PubSubClient.h>

#include "en_app.h"
#include "en_main.h"

#include "gui/mainbar/app_tile/app_tile.h"
#include "gui/mainbar/main_tile/main_tile.h"
#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/app.h"
#include "gui/widget.h"
#include "gui/widget_factory.h"
#include "gui/widget_styles.h"

#include "hardware/wifictl.h"

#include "utils/json_psram_allocator.h"
#include "utils/alloc.h"

lv_obj_t *en_main_tile = NULL;
lv_style_t en_main_style;
lv_style_t en_id_style;

lv_task_t * _en_main_task;

lv_obj_t *id_cont = NULL;
lv_obj_t *id_label = NULL;
lv_obj_t *voltage_cont = NULL;
lv_obj_t *voltage_label = NULL;
lv_obj_t *current_cont = NULL;
lv_obj_t *current_label = NULL;
lv_obj_t *power_cont = NULL;
lv_obj_t *power_label = NULL;

WiFiClient espClient;
PubSubClient en_mqtt_client( espClient );

LV_IMG_DECLARE(refresh_32px);
LV_FONT_DECLARE(Ubuntu_16px);
LV_FONT_DECLARE(Ubuntu_48px);

bool en_wifictl_event_cb( EventBits_t event, void *arg );
void en_main_task( lv_task_t * task );

void callback(char* topic, byte* payload, unsigned int length) {

    String topicStr(topic);
    String subTopic = topicStr.substring(topicStr.indexOf('/'));
    log_i("subtopic: %s", subTopic.c_str());
    
    
    if ( subTopic == "A" ) {
        String msg;
        if (payload[0] == 1 || payload[0] == '1')
            msg = "true";
        else
            msg = "false";
        lv_label_set_text( power_label, msg.c_str() );
    }

    lv_obj_align( id_label, id_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_align( power_label, power_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_align( voltage_label, voltage_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_align( current_label, current_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
}

void en_main_tile_setup( uint32_t tile_num ) {

    en_main_tile = mainbar_get_tile_obj( tile_num );

    lv_style_copy( &en_main_style, ws_get_app_opa_style() );
    lv_style_set_text_font( &en_main_style, LV_STATE_DEFAULT, &Ubuntu_48px);
    lv_obj_add_style( en_main_tile, LV_OBJ_PART_MAIN, &en_main_style );

    lv_style_copy( &en_id_style, ws_get_app_opa_style() );
    lv_style_set_text_font( &en_id_style, LV_STATE_DEFAULT, &Ubuntu_16px);

    lv_obj_t * exit_btn = wf_add_exit_button( en_main_tile, &en_main_style );
    lv_obj_align(exit_btn, en_main_tile, LV_ALIGN_IN_BOTTOM_LEFT, 10, -10 );


    id_cont = lv_obj_create( en_main_tile, NULL );
    lv_obj_set_size( id_cont, lv_disp_get_hor_res( NULL ), 20 );
    lv_obj_add_style( id_cont, LV_OBJ_PART_MAIN, &en_id_style );
    lv_obj_align( id_cont, en_main_tile, LV_ALIGN_IN_TOP_MID, 0, 0 );
    lv_obj_t * id_info_label = lv_label_create( id_cont, NULL );
    lv_obj_add_style( id_info_label, LV_OBJ_PART_MAIN, &en_id_style );
    lv_label_set_text( id_info_label, "ID:" );
    lv_obj_align( id_info_label, id_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    id_label = lv_label_create( id_cont, NULL );
    lv_obj_add_style( id_label, LV_OBJ_PART_MAIN, &en_id_style );
    lv_label_set_text( id_label, "n/a" );
    lv_obj_align( id_label, id_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );

    voltage_cont = lv_obj_create( en_main_tile, NULL );
    lv_obj_set_size( voltage_cont, lv_disp_get_hor_res( NULL ), 56 );
    lv_obj_add_style( voltage_cont, LV_OBJ_PART_MAIN, &en_main_style );
    lv_obj_align( voltage_cont, id_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t * voltage_info_label = lv_label_create( voltage_cont, NULL );
    lv_obj_add_style( voltage_info_label, LV_OBJ_PART_MAIN, &en_main_style );
    lv_label_set_text( voltage_info_label, "U =" );
    lv_obj_align( voltage_info_label, voltage_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    voltage_label = lv_label_create( voltage_cont, NULL );
    lv_obj_add_style( voltage_label, LV_OBJ_PART_MAIN, &en_main_style );
    lv_label_set_text( voltage_label, "n/a" );
    lv_obj_align( voltage_label, voltage_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );

    current_cont = lv_obj_create( en_main_tile, NULL );
    lv_obj_set_size( current_cont, lv_disp_get_hor_res( NULL ), 56 );
    lv_obj_add_style( current_cont, LV_OBJ_PART_MAIN, &en_main_style );
    lv_obj_align( current_cont, voltage_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t * current_info_label = lv_label_create( current_cont, NULL );
    lv_obj_add_style( current_info_label, LV_OBJ_PART_MAIN, &en_main_style );
    lv_label_set_text( current_info_label, "I =" );
    lv_obj_align( current_info_label, current_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    current_label = lv_label_create( current_cont, NULL );
    lv_obj_add_style( current_label, LV_OBJ_PART_MAIN, &en_main_style );
    lv_label_set_text( current_label, "n/a" );
    lv_obj_align( current_label, current_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );

    power_cont = lv_obj_create( en_main_tile, NULL );
    lv_obj_set_size( power_cont, lv_disp_get_hor_res( NULL ), 56 );
    lv_obj_add_style( power_cont, LV_OBJ_PART_MAIN, &en_main_style );
    lv_obj_align( power_cont, current_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t * power_info_label = lv_label_create( power_cont, NULL );
    lv_obj_add_style( power_info_label, LV_OBJ_PART_MAIN, &en_main_style );
    lv_label_set_text( power_info_label, "P =" );
    lv_obj_align( power_info_label, power_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    power_label = lv_label_create( power_cont, NULL );
    lv_obj_add_style( power_label, LV_OBJ_PART_MAIN, &en_main_style );
    lv_label_set_text( power_label, "n/a" );
    lv_obj_align( power_label, power_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );

    en_mqtt_client.setCallback( callback );
    en_mqtt_client.setBufferSize( 512 );

    wifictl_register_cb( WIFICTL_CONNECT_IP | WIFICTL_OFF_REQUEST | WIFICTL_OFF | WIFICTL_DISCONNECT , en_wifictl_event_cb, "en" );
    // create an task that runs every secound
    _en_main_task = lv_task_create( en_main_task, 250, LV_TASK_PRIO_MID, NULL );
}

bool en_wifictl_event_cb( EventBits_t event, void *arg ) {
    en_config_t *en_config = en_get_config();
    switch( event ) {
        case WIFICTL_CONNECT_IP:    if ( en_config->autoconnect ) {
                                        en_mqtt_client.setServer( en_config->server, en_config->port );
                                        if ( !en_mqtt_client.connect( "en", en_config->user, en_config->password ) ) {
                                            log_e("connect to mqtt server %s failed", en_config->server );
                                            app_set_indicator( en_get_app_icon(), ICON_INDICATOR_FAIL );
                                            widget_set_indicator( en_get_widget_icon() , ICON_INDICATOR_FAIL );
                                        }
                                        else {
                                            log_i("connect to mqtt server %s success", en_config->server );
                                            en_mqtt_client.subscribe( en_config->topic );
                                            app_set_indicator( en_get_app_icon(), ICON_INDICATOR_OK );
                                            widget_set_indicator( en_get_widget_icon(), ICON_INDICATOR_OK );
                                        }
                                    } 
                                    break;
        case WIFICTL_OFF_REQUEST:
        case WIFICTL_OFF:
        case WIFICTL_DISCONNECT:    if ( en_mqtt_client.connected() ) {
                                        log_i("disconnect from mqtt server %s", en_config->server );
                                        en_mqtt_client.disconnect();
                                        app_hide_indicator( en_get_app_icon() );
                                        widget_hide_indicator( en_get_widget_icon() );
                                        widget_set_label( en_get_widget_icon(), "n/a" );
                                    }
                                    break;
    }
    return( true );
}


void en_main_task( lv_task_t * task ) {
    // put your code her
    en_mqtt_client.loop();
}