#pragma once

#define send_message(severity, ...) _api->_send_debug_message(severity, _AP_FUNCTION, __VA_ARGS__)
#define report_invalid_enum(enum_value) send_message(G_DEBUG_MESSAGE_SEVERITY_ERROR, "Invalid graphics enum: %s", get_graphics_enum_string(enum_value))