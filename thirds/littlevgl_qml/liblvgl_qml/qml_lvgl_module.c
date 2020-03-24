/****************************************************************************
**
** Copyright (C) 2020 @scriptiot
**
**  EVM是一款通用化设计的虚拟机引擎，拥有语法解析前端接口、编译器、虚拟机和虚拟机扩展接口框架。
**  支持js、python、qml、lua等多种脚本语言，纯Ｃ开发，零依赖，支持主流 ROM > 50KB, RAM > 2KB的MCU;
**  自带垃圾回收（GC）先进的内存管理，采用最复杂的压缩算法，无内存碎片（大部分解释器都存在内存碎片）
**  Version : 1.0
**  Email   : scriptiot@aliyun.com
**  Website : https://github.com/scriptiot
**  Licence: MIT Licence
****************************************************************************/
#include "qml_lvgl_utils.h"

/********** Global Variables ***********/

static evm_t * global_e;

/********** GC ***********/

static void lv_qml_gc_init(evm_t * e, evm_val_t *old_self, evm_val_t * new_self){
    lv_obj_t * o = (lv_obj_t*)evm_qml_object_get_pointer(old_self);
    lv_obj_set_user_data(o, (lv_obj_user_data_t)*new_self);
    qml_object_gc_init(e, old_self, new_self);
}

/********** Callback ***********/

static void lv_qml_run_callback(evm_t * e, evm_val_t * obj, char * name, int argc, evm_val_t * argv){
    evm_val_t * m_clicked_fn = evm_prop_get(e, obj, name, 0);
    if( m_clicked_fn != NULL)
        evm_run_callback(e, m_clicked_fn, obj, argv, argc);
}

static void lv_qml_Button_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        evm_val_t v = (evm_val_t)lv_obj_get_user_data(obj);
        lv_qml_run_callback(global_e, &v, "onClicked", 0, NULL);
    }else if(event == LV_EVENT_PRESSED) {
        evm_val_t v = (evm_val_t)lv_obj_get_user_data(obj);
        lv_qml_run_callback(global_e, &v, "onPressed", 0, NULL);
    }else if(event == LV_EVENT_RELEASED) {
        evm_val_t v = (evm_val_t)lv_obj_get_user_data(obj);
        lv_qml_run_callback(global_e, &v, "onReleased", 0, NULL);
    }
}

static void lv_qml_TextInput_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        evm_val_t v = (evm_val_t)lv_obj_get_user_data(obj);
        evm_qml_write_value(global_e, &v, "text", evm_mk_foreign_string((intptr_t)lv_ta_get_text(obj)));
        lv_qml_run_callback(global_e, &v, "onTextEdited", 0, NULL);
    }
}

/********** Item ***********/

static evm_val_t  qml_Item_x(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_number(v)){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer(p);
        lv_obj_set_x(obj, evm_2_integer(v));
    }
    return EVM_VAL_UNDEFINED;
}

static evm_val_t  qml_Item_y(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_number(v)){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer(p);
        lv_obj_set_y(obj, evm_2_integer(v));
    }
    return EVM_VAL_UNDEFINED;
}

static evm_val_t  qml_Item_width(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_number(v)){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer(p);
        lv_obj_set_width(obj, evm_2_integer(v));
    }
    return EVM_VAL_UNDEFINED;
}

static evm_val_t  qml_Item_height(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_number(v)){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer(p);
        lv_obj_set_height(obj, evm_2_integer(v));
    }
    return EVM_VAL_UNDEFINED;
}

static evm_val_t  qml_Item_opacity(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_number(v)){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer(p);
        lv_style_t * style = (lv_style_t*)lv_obj_get_style(obj);
        style->body.opa = 255 * evm_2_double(v);
        lv_obj_refresh_style(obj);
    }
    return EVM_VAL_UNDEFINED;
}

static evm_val_t  qml_Item_visible(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_boolean(v)){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer(p);
        lv_obj_set_hidden(obj, evm_2_intptr(v));
    }
    return EVM_VAL_UNDEFINED;
}

/********** Rectangle ***********/

static evm_val_t  qml_Rectangle(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    lv_obj_t * obj = NULL;
    lv_obj_t * parent = lv_scr_act();
    if(argc == 1){
        parent = evm_qml_object_get_pointer(v);
    }
    obj = (lv_obj_t*)lv_obj_create(parent, NULL);
    if( !obj ) return EVM_VAL_UNDEFINED;
    evm_qml_object_set_pointer(p, obj);
    lvgl_qml_obj_add_style(obj);
    return EVM_VAL_UNDEFINED;
}

static evm_val_t  qml_Rectangle_color(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer(p);
        lv_style_t * style = (lv_style_t*)lv_obj_get_style(obj);
        style->body.main_color = lvgl_qml_style_get_color(e, v);
        lv_obj_refresh_style(obj);
    }
    return EVM_VAL_UNDEFINED;
}

static evm_val_t  qml_Rectangle_gradient(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer(p);
        lv_style_t * style = (lv_style_t*)lv_obj_get_style(obj);
        style->body.grad_color = lvgl_qml_style_get_color(e, v);
        lv_obj_refresh_style(obj);
    }
    return EVM_VAL_UNDEFINED;
}

static evm_val_t  qml_Rectangle_radius(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_number(v)){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer(p);
        lv_style_t * style = (lv_style_t*)lv_obj_get_style(obj);
        style->body.radius = evm_2_integer(v);
        lv_obj_refresh_style(obj);
    }
    return EVM_VAL_UNDEFINED;
}

/********** Button ***********/

static evm_val_t  qml_Button(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    lv_obj_t * obj = NULL;
    lv_obj_t * parent = lv_scr_act();
    if(argc == 1) parent = evm_qml_object_get_pointer(v);
    obj = (lv_obj_t*)lv_btn_create(parent, NULL);
    if( !obj ) return EVM_VAL_UNDEFINED;
    evm_qml_object_set_pointer(p, obj);
    lv_obj_set_event_cb(obj, lv_qml_Button_event_handler);
    evm_object_set_init(p, (evm_init_fn)lv_qml_gc_init);
    lv_obj_set_user_data(obj, (lv_obj_user_data_t)*p);
    lvgl_qml_obj_add_style(obj);
    return EVM_VAL_UNDEFINED;
}

/********** Text ***********/

static evm_val_t  qml_Text(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    lv_obj_t * obj = NULL;
    lv_obj_t * parent = lv_scr_act();
    if(argc == 1) parent = evm_qml_object_get_pointer(v);
    obj = (lv_obj_t*)lv_label_create(parent, NULL);
    if( !obj ) return EVM_VAL_UNDEFINED;
    evm_qml_object_set_pointer(p, obj);
    lvgl_qml_obj_add_style(obj);
    return EVM_VAL_UNDEFINED;
}

static evm_val_t  qml_Text_color(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer(p);
        lv_style_t * style = (lv_style_t*)lv_obj_get_style(obj);
        style->text.color = lvgl_qml_style_get_color(e, v);
        lv_obj_refresh_style(obj);
    }
    return EVM_VAL_UNDEFINED;
}

static evm_val_t  qml_Text_text(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_string(v) ){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer(p);
        lv_label_set_text(obj, evm_2_string(v));
    }
    return EVM_VAL_UNDEFINED;
}

/********** Image ***********/

static evm_val_t  qml_Image(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    lv_obj_t * obj = NULL;
    lv_obj_t * parent = lv_scr_act();
    if(argc == 1) parent = evm_qml_object_get_pointer(v);
    obj = (lv_obj_t*)lv_img_create(parent, NULL);
    if( !obj ) return EVM_VAL_UNDEFINED;
    evm_qml_object_set_pointer(p, obj);
    lvgl_qml_obj_add_style(obj);
    return EVM_VAL_UNDEFINED;
}

static evm_val_t  qml_Image_source(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_string(v) ){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer(p);
        char img[evm_string_len(v) + 2];
        sprintf(img, "P:%s", evm_2_string(v));
        lv_img_set_src(obj, img);
    }
    return EVM_VAL_UNDEFINED;
}

/********** CheckBox ***********/

static evm_val_t  qml_CheckBox(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    lv_obj_t * obj = NULL;
    lv_obj_t * parent = lv_scr_act();
    if(argc == 1) parent = evm_qml_object_get_pointer(v);
    obj = (lv_obj_t*)lv_cb_create(parent, NULL);
    if( !obj ) return EVM_VAL_UNDEFINED;
    evm_qml_object_set_pointer(p, obj);
    lvgl_qml_obj_add_style(obj);
    lv_obj_set_event_cb(obj, lv_qml_Button_event_handler);
    evm_object_set_init(p, (evm_init_fn)lv_qml_gc_init);
    lv_obj_set_user_data(obj, (lv_obj_user_data_t)*p);
    return EVM_VAL_UNDEFINED;
}

static evm_val_t  qml_CheckBox_text(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_string(v) ){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer(p);
        lv_cb_set_text(obj, evm_2_string(v));
    }
    return EVM_VAL_UNDEFINED;
}

static evm_val_t  qml_CheckBox_checked(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_boolean(v) ){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer(p);
        lv_cb_set_checked(obj, evm_2_intptr(v));
    }
    return EVM_VAL_UNDEFINED;
}

/********** ScrollView ***********/

static evm_val_t  qml_ScrollView(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    lv_obj_t * obj = NULL;
    lv_obj_t * parent = lv_scr_act();
    if(argc == 1) parent = evm_qml_object_get_pointer(v);
    obj = (lv_obj_t*)lv_page_create(parent, NULL);
    if( !obj ) return EVM_VAL_UNDEFINED;
    evm_qml_object_set_pointer(p, obj);
    lvgl_qml_obj_add_style(obj);
    return EVM_VAL_UNDEFINED;
}

/********** TextField ***********/

static evm_val_t  qml_TextField(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    lv_obj_t * obj = NULL;
    lv_obj_t * parent = lv_scr_act();
    if(argc == 1) parent = evm_qml_object_get_pointer(v);
    obj = (lv_obj_t*)lv_ta_create(parent, NULL);
    if( !obj ) return EVM_VAL_UNDEFINED;
    evm_qml_object_set_pointer(p, obj);
    lv_obj_set_user_data(obj, (lv_obj_user_data_t)*p);
    evm_object_set_init(p, (evm_init_fn)lv_qml_gc_init);
    lv_ta_set_cursor_type(obj, LV_CURSOR_LINE | LV_CURSOR_HIDDEN);
    lv_obj_set_event_cb(obj, lv_qml_TextInput_event_handler);
    lvgl_qml_obj_add_style(obj);
    lv_ta_set_one_line(obj, 1);
    return EVM_VAL_UNDEFINED;
}

static evm_val_t  qml_TextField_text(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_string(v) ){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer(p);
        lv_ta_set_text(obj, evm_2_string(v));
    }
    return EVM_VAL_UNDEFINED;
}

static evm_val_t  qml_TextField_placeholderText(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_string(v) ){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer(p);
        lv_ta_set_placeholder_text(obj, evm_2_string(v));
    }
    return EVM_VAL_UNDEFINED;
}

/********** CircularGauge ***********/

static evm_val_t  qml_CircularGauge(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    lv_obj_t * obj = NULL;
    lv_obj_t * parent = lv_scr_act();
    if(argc == 1) parent = evm_qml_object_get_pointer(v);
    obj = (lv_obj_t*)lv_gauge_create(parent, NULL);
    if( !obj ) return EVM_VAL_UNDEFINED;
    evm_qml_object_set_pointer(p, obj);
    lvgl_qml_obj_add_style(obj);
    return EVM_VAL_UNDEFINED;
}

static evm_val_t qml_CircularGauge_value(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_number(v)){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer( p );
        lv_gauge_set_value(obj, 0, evm_2_integer(v));
    }
    return EVM_VAL_UNDEFINED;
}

static evm_val_t qml_CircularGauge_minimumValue(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_number(v)){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer( p );
        int max = lv_gauge_get_max_value(obj);
        lv_gauge_set_range(obj, evm_2_integer(v), max);
    }
    return EVM_VAL_UNDEFINED;
}

static evm_val_t qml_CircularGauge_maximumValue(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_number(v)){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer( p );
        int min = lv_gauge_get_min_value(obj);
        lv_gauge_set_range(obj, min, evm_2_integer(v));
    }
    return EVM_VAL_UNDEFINED;
}

/********** CircularGaugeStyle ***********/

static evm_val_t  qml_CircularGaugeStyle(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    lv_obj_t * parent = lv_scr_act();
    if(argc == 1) parent = evm_qml_object_get_pointer(v);
    evm_qml_object_set_pointer(p, parent);
    return EVM_VAL_UNDEFINED;
}

static evm_val_t qml_CircularGaugeStyle_angleRange(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_number(v)){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer( p );
        uint8_t line_cnt = lv_gauge_get_line_count(obj);
        uint8_t label_cnt = lv_gauge_get_label_count(obj);
        lv_gauge_set_scale(obj, evm_2_integer(v), line_cnt, label_cnt);
    }
    return EVM_VAL_UNDEFINED;
}

static evm_val_t qml_CircularGaugeStyle_labelCount(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_number(v)){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer( p );
        uint16_t angle = lv_gauge_get_scale_angle(obj);
        uint8_t line_cnt = lv_gauge_get_line_count(obj);
        lv_gauge_set_scale(obj, angle, line_cnt, evm_2_integer(v));
    }
    return EVM_VAL_UNDEFINED;
}

static evm_val_t qml_CircularGaugeStyle_tickmarkCount(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_number(v)){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer( p );
        uint16_t angle = lv_gauge_get_scale_angle(obj);
        uint8_t lable_cnt = lv_gauge_get_label_count(obj);
        lv_gauge_set_scale(obj, angle, evm_2_integer(v), lable_cnt);
    }
    return EVM_VAL_UNDEFINED;
}

static evm_val_t qml_CircularGaugeStyle_needleColor(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    if( argc >= 1){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer( p );
        lv_color_t * colors = lv_mem_alloc(sizeof(lv_color_t));
        colors[0] = lvgl_qml_style_get_color(e, v);
        lv_gauge_set_needle_count(obj, 1, colors);
    }
    return EVM_VAL_UNDEFINED;
}

/********** Window ***********/

static evm_val_t  qml_Window(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    lv_obj_t * obj = NULL;
    lv_obj_t * parent = lv_scr_act();
    if(argc == 1) parent = evm_qml_object_get_pointer(v);
    obj = (lv_obj_t*)lv_win_create(parent, NULL);
    if( !obj ) return EVM_VAL_UNDEFINED;
    evm_qml_object_set_pointer(p, obj);
    lvgl_qml_obj_add_style(obj);
    return EVM_VAL_UNDEFINED;
}

static evm_val_t qml_Window_title(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_string(v) ){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer( p );
        lv_win_set_title(obj, evm_2_string(v));
    }
    return EVM_VAL_UNDEFINED;
}

/********** border ***********/

static evm_val_t qml_border_width(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 && evm_is_number(v) ){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer( evm_get_parent(e, *p) );
        lv_style_t * style = (lv_style_t*)lv_obj_get_style(obj);
        style->body.border.width = evm_2_integer(v);
        lv_obj_refresh_style(obj);
    }
    return EVM_VAL_UNDEFINED;
}

static evm_val_t qml_border_color(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);
    if( argc >= 1 ){
        lv_obj_t * obj = (lv_obj_t*)evm_qml_object_get_pointer( evm_get_parent(e, *p) );
        lv_style_t * style = (lv_style_t*)lv_obj_get_style(obj);
        style->body.border.color = lvgl_qml_style_get_color(e, v);
        lv_obj_refresh_style(obj);
    }
    return EVM_VAL_UNDEFINED;
}

/********** Properties ***********/

#define BUTTON_CALLBACKS    {EVM_QML_CALLBACK, "onClicked", NULL},\
                            {EVM_QML_CALLBACK, "onPressed", NULL},\
                            {EVM_QML_CALLBACK, "onReleased", NULL},

static evm_qml_value_reg_t qml_properties_Item[] = {
    {EVM_QML_INT, "x", (evm_native_fn)qml_Item_x},
    {EVM_QML_INT, "y", (evm_native_fn)qml_Item_y},
    {EVM_QML_INT, "width", (evm_native_fn)qml_Item_width},
    {EVM_QML_INT, "height", (evm_native_fn)qml_Item_height},
    {EVM_QML_DOUBLE, "opacity", (evm_native_fn)qml_Item_opacity},
    {EVM_QML_BOOLEAN, "visible", (evm_native_fn)qml_Item_visible},
    {0, NULL, NULL}
};

static evm_qml_value_reg_t qml_properties_border[] = {
    {EVM_QML_INT, "width", (evm_native_fn)qml_border_width},
    {EVM_QML_INT|EVM_QML_STRING, "color", (evm_native_fn)qml_border_color},
    {0, NULL, NULL}
};

static evm_qml_value_reg_t qml_properties_Rectangle[] = {
    {EVM_QML_INT|EVM_QML_STRING, "color", (evm_native_fn)qml_Rectangle_color},
    {EVM_QML_INT|EVM_QML_STRING, "gradient", (evm_native_fn)qml_Rectangle_gradient},
    {EVM_QML_INT, "radius", (evm_native_fn)qml_Rectangle_radius},
    {EVM_QML_GROUP, "border", qml_properties_border},
    {0, NULL, NULL}
};

static evm_qml_value_reg_t qml_properties_Button[] = {
    BUTTON_CALLBACKS
    {0, NULL, NULL}
};

static evm_qml_value_reg_t qml_properties_Text[] = {
    {EVM_QML_INT|EVM_QML_STRING, "color", (evm_native_fn)qml_Text_color},
    {EVM_QML_STRING, "text", (evm_native_fn)qml_Text_text},
    {0, NULL, NULL}
};

static evm_qml_value_reg_t qml_properties_Image[] = {
    {EVM_QML_STRING, "source", (evm_native_fn)qml_Image_source},
    {0, NULL, NULL}
};

static evm_qml_value_reg_t qml_properties_CheckBox[] = {
    BUTTON_CALLBACKS
    {EVM_QML_STRING, "text", (evm_native_fn)qml_CheckBox_text},
    {EVM_QML_BOOLEAN, "checked", (evm_native_fn)qml_CheckBox_checked},
    {0, NULL, NULL}
};

static evm_qml_value_reg_t qml_properties_ScrollView[] = {
    {0, NULL, NULL}
};

static evm_qml_value_reg_t qml_properties_TextField[] = {
    {EVM_QML_STRING, "text", (evm_native_fn)qml_TextField_text},
    {EVM_QML_STRING, "placeholderText", (evm_native_fn)qml_TextField_placeholderText},
    {EVM_QML_CALLBACK, "onTextEdited", NULL},
    {0, NULL, NULL}
};

static evm_qml_value_reg_t qml_properties_CircularGauge[] = {
    {EVM_QML_INT, "value", (evm_native_fn)qml_CircularGauge_value},
    {EVM_QML_INT, "maximumValue", (evm_native_fn)qml_CircularGauge_maximumValue},
    {EVM_QML_INT, "minimumValue", (evm_native_fn)qml_CircularGauge_minimumValue},
    {0, NULL, NULL}
};

static evm_qml_value_reg_t qml_properties_CircularGaugeStyle[] = {
    {EVM_QML_INT, "angleRange", (evm_native_fn)qml_CircularGaugeStyle_angleRange},
    {EVM_QML_INT, "labelCount", (evm_native_fn)qml_CircularGaugeStyle_labelCount},
    {EVM_QML_INT, "tickmarkCount", (evm_native_fn)qml_CircularGaugeStyle_tickmarkCount},
    {EVM_QML_INT|EVM_QML_STRING, "needleColor", (evm_native_fn)qml_CircularGaugeStyle_needleColor},
    {0, NULL, NULL}
};

static evm_qml_value_reg_t qml_properties_Window[] = {
    {EVM_QML_STRING, "title", (evm_native_fn)qml_Window_title},
    {0, NULL, NULL}
};

static evm_val_t qml_lv_init(evm_t * e, evm_val_t *p, int argc, evm_val_t * v){
    EVM_UNUSED(e);EVM_UNUSED(argc);EVM_UNUSED(v);
    int len = evm_prop_len(p);
    for(int i = 0; i < len; i++){
        evm_val_t * obj = evm_prop_get_by_index(e, p, i);
        if( evm_is_class(obj) || evm_is_object(obj) ){
            evm_val_t * fn = evm_prop_get(e, obj, "onCompleted", 0);
            if(fn){
                evm_run_callback(e, fn, obj, NULL, 0);
            }
        }
    }
    return EVM_VAL_UNDEFINED;
}

int qml_lvgl_module(evm_t * e){
    global_e = e;
    evm_qml_object_reg_t qml_objects[] = {
        {"Item", NULL, NULL, qml_properties_Item},
        {"Rectangle", "Item", (evm_native_fn)qml_Rectangle, qml_properties_Rectangle},
        {"Text", "Item", (evm_native_fn)qml_Text, qml_properties_Text},
        {"Button", "Item", (evm_native_fn)qml_Button, qml_properties_Button},
        {"Image", "Item", (evm_native_fn)qml_Image, qml_properties_Image},
        {"CheckBox", "Item", (evm_native_fn)qml_CheckBox, qml_properties_CheckBox},
        {"ScrollView", "Item", (evm_native_fn)qml_ScrollView, qml_properties_ScrollView},
        {"TextField", "Item", (evm_native_fn)qml_TextField, qml_properties_TextField},
        {"CircularGauge", "Item", (evm_native_fn)qml_CircularGauge, qml_properties_CircularGauge},
        {"CircularGaugeStyle", NULL, (evm_native_fn)qml_CircularGaugeStyle, qml_properties_CircularGaugeStyle},
        {"Window", "Item", (evm_native_fn)qml_Window, qml_properties_Window},
        {NULL, NULL, NULL, NULL}
    };

    evm_qml_register(e, qml_objects);
    int err = qml_module(e, (evm_native_fn)qml_lv_init);
    return err;
}

