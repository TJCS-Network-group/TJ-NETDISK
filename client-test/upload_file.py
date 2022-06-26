import requests
from hashlib import md5
import json
import os

test_url = "http://192.168.80.230:7777"  #url

headers = {}


def get_md5(string):
    return md5(string).hexdigest()


def get_file_md5(file_path):
    with open(file_path, "rb") as fp:
        return get_md5(fp.read())


def login():
    """先登录"""
    url = f"{test_url}/api/login"
    payload = json.dumps({"user_name": "test", "password": "test"})
    headers['Content-Type'] = 'application/json'
    response = requests.request("POST", url, headers=headers, data=payload)
    headers.pop("Content-Type")
    headers["Cookie"] = (response.headers["Set-Cookie"])


def get_file_exist(file_path):
    """文件是否存在"""
    url = f"{test_url}/api/file_exist" + "?md5=" + get_file_md5(file_path)
    response = requests.request("GET", url, headers=headers)
    return response.json()["data"]["is_exist"]


def get_file_fragment_exist(file_path):
    """文件碎片是否存在"""
    url = f"{test_url}/api/file_fragment_exist" + "?md5=" + get_file_md5(
        file_path)
    response = requests.request("GET", url, headers=headers)
    return response.json()["data"]["is_exist"]


def upload_fragment(fragment_path, index, file_md5, post_fragment):
    """上传碎片"""
    payload = {
        "index": index,
        "file_md5": file_md5,
        "fragment_md5": get_file_md5(fragment_path)
    }
    url = f"{test_url}/api/upload_fragment"
    if post_fragment:
        files = [('file_fragment', ('fragment_name', open(fragment_path,
                                                          'rb')))]
        response = requests.request("POST",
                                    url,
                                    headers=headers,
                                    data=payload,
                                    files=files)
    else:
        response = requests.request("POST", url, headers=headers, data=payload)
    print("上传文件碎片：", response.json())


def get_root_id():
    url = f"{test_url}/api/get_root_id"
    response = requests.request("GET", url, headers=headers)
    return response.json()["data"]["root_id"]


def upload_file(filename, file_md5, parent_dir_id=2):
    """上传文件"""
    url = f"{test_url}/api/upload_file"
    payload = json.dumps({
        "filename": filename,
        "md5": file_md5,
        "parent_dir_id": parent_dir_id
    })
    headers['Content-Type'] = 'application/json'
    response = requests.request("POST", url, headers=headers, data=payload)
    headers.pop("Content-Type")
    print("上传文件：", response.json())


def get_upload_allocation(md5, size):
    """获取要上传的文件块号"""
    url = f"{test_url}/api/upload_allocation" + "?md5=" + md5 + "&size=" + str(
        size)
    response = requests.request("GET", url, headers=headers)
    return response.json()["data"]["next_index"]


import random


def main():
    filename = "te.pdf"
    file_path = "./" + filename
    login()
    if get_file_exist(file_path) == True:
        upload_file(filename, get_file_md5(file_path), get_root_id())
    else:
        fragment_map = {}
        fsize = os.stat(file_path).st_size
        with open(file_path, "rb") as f:
            cur_index = 0
            while fsize > f.tell():
                fgsize = min(fsize - f.tell(), 4 * 1024 * 1024)
                tmp = f.read(fgsize)
                fg_md5 = get_md5(tmp)
                fragment_map[cur_index] = fg_md5
                with open("./pool/{}".format(fg_md5), "wb") as fp:
                    fp.write(tmp)
                cur_index += 1

        next_index = get_upload_allocation(get_file_md5(file_path), fsize)
        while next_index != -1:
            print("next_index:", next_index)
            fragment_exist = get_file_fragment_exist(
                f"./pool/{fragment_map[next_index]}")
            if random.randint(0, 4) > 0:
                upload_fragment(f"./pool/{fragment_map[next_index]}",
                                next_index, get_file_md5(file_path),
                                not fragment_exist)
            else:
                print("该碎片由于网速问题未上传成功")
            next_index = get_upload_allocation(get_file_md5(file_path), fsize)
        upload_file(filename, get_file_md5(file_path), get_root_id())


if __name__ == "__main__":
    main()