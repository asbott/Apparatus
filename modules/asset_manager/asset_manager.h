#pragma once

#include "apparatus.h"

typedef u32 asset_request_id_t;
typedef u32 asset_id_t;
typedef size_t data_index_t;
typedef s32 asset_type_t;

constexpr asset_type_t ASSET_TYPE_NONE = 0;
constexpr asset_type_t ASSET_TYPE_TEXTURE = 1;

constexpr data_index_t NULL_DATA_INDEX = (data_index_t)-1;
constexpr asset_id_t NULL_ASSET_ID = (asset_id_t)-1;

constexpr asset_request_id_t ASSET_REQUEST_REGISTER_ASSET = 1;
constexpr asset_request_id_t ASSET_REQUEST_BEGIN_USE_ASSET = 2;
constexpr asset_request_id_t ASSET_REQUEST_END_USE_ASSET = 3;
constexpr asset_request_id_t ASSET_REQUEST_CHECK_IF_VALID = 4;
constexpr asset_request_id_t ASSET_REQUEST_VIEW = 5;

struct Asset_Request {
	Asset_Request(asset_request_id_t r) : request_id(r) {}
	asset_request_id_t request_id;
};

struct Asset_Request_Register_Asset : public Asset_Request {
	Asset_Request_Register_Asset() : Asset_Request(ASSET_REQUEST_REGISTER_ASSET) {}

	asset_type_t asset_type = ASSET_TYPE_NONE;
	path_str_t path;
};

struct Asset_Request_Begin_Use_asset : public Asset_Request {
	Asset_Request_Begin_Use_asset() : Asset_Request(ASSET_REQUEST_BEGIN_USE_ASSET) {}

	asset_id_t asset_id;
};

struct Asset_Request_End_Use_asset : public Asset_Request {
	Asset_Request_End_Use_asset() : Asset_Request(ASSET_REQUEST_END_USE_ASSET) {}

	asset_id_t asset_id;
};

struct Asset_Request_Check_If_Valid : public Asset_Request {
	Asset_Request_Check_If_Valid() : Asset_Request(ASSET_REQUEST_CHECK_IF_VALID) {}

	asset_id_t asset_id;
};

struct Asset_Request_View : public Asset_Request {
	Asset_Request_View() : Asset_Request(ASSET_REQUEST_VIEW) {}

	asset_id_t asset_id;
};



struct Texture_Data {
	graphics_id_t graphics_id;
	ivec2 size;
	s32 channels;
	void* data;
};

struct Asset {
	path_str_t path = "";
	name_str_t file_name = "";
	name_str_t name = "";
	str16_t extension = "";

	asset_type_t asset_type;

	Dynamic_Array<byte>* data_stream;
	bool in_memory = false;

	u32 usage_points = 0;

	bool in_use = false;
	data_index_t data_index;

	bool is_garbage = false;

	inline void* get_runtime_data() {
		return &(*data_stream)[data_index];
	}
};