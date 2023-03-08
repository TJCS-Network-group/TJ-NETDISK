#include "../include/HttpServer.h"

//初始化路由表
void Routers::Init_routers()
{
    setRouter("POST", "/api/login", POST_login);
    setRouter("GET", "/api/logout", GET_logout);
    setRouter("POST", "/api/register", POST_register);
    setRouter("GET", "/api/filesystem/get_dir", GET_filesystem_get_dir);
    setRouter("GET", "/api/get_root_id", GET_get_root_id);
    setRouter("GET", "/api/file_exist", GET_file_exist);
    setRouter("GET", "/api/file_fragment_exist", GET_file_fragment);
    setRouter("GET", "/api/upload_allocation", GET_upload_allocation);
    setRouter("PUT", "/api/filesystem/rename_file", PUT_filesystem_rename_file);
    setRouter("PUT", "/api/filesystem/rename_dir", PUT_filesystem_rename_dir);
    setRouter("GET", "/api/download_fragment", GET_download_fragment);
    setRouter("POST", "/api/filesystem/create_dir", POST_filesystem_create_dir);
    setRouter("POST", "/api/upload_fragment", POST_upload_fragment);
    setRouter("POST", "/api/upload_file", POST_upload_file);
    setRouter("POST", "/api/share/move_file", POST_share_move_file);
    setRouter("POST", "/api/share/copy_file", POST_share_copy_file);
    setRouter("POST", "/api/share/move_dir", POST_share_move_dir);
    setRouter("POST", "/api/share/copy_dir", POST_share_copy_dir);
    setRouter("DELETE", "/api/remove_file", DEL_remove_file);
    setRouter("DELETE", "/api/remove_dir", DEL_remove_dir);
    setRouter("GET", "/api/share/get_dir_tree", GET_share_get_dir_tree);
    setRouter("GET", "/api/filesystem/get_tree", GET_filesystem_get_tree);
    setRouter("GET", "/api/myinfo", GET_myinfo);
    setRouter("GET", "/api/get_identity", GET_get_identity);
    setRouter("GET", "/api/filesystem/get_md5code", GET_filesystem_get_md5code);
}
