# -*- coding: gbk -*-
from copy import deepcopy
import os
import hashlib
import pymysql as pm
pool_path='../pool'
db = pm.connect(host='localhost',
                user='root',
                password='root123',
                charset="gbk")

db_cursor = db.cursor()
with open("pan.sql", "r", encoding='gbk') as f:
    x = list(map(lambda x: x.strip(), f.readlines()))
    t = list()
    p = ""
    for i in x:
        p += deepcopy(i)
        if p[-1] == ";":
            t.append(p)
            p = ""
    for i in t:
        db_cursor.execute(i)
        db.commit()
db_cursor.close()
db.close()
db = pm.connect(host='localhost',
                user='root',
                password='root123',
                database="pan",
                charset="gbk")
db_cursor = db.cursor()
for j in range(1, 4):
    sql = "insert into DirectoryEntity(id,dname,parent_id) value {};".format(
        (j, "root", j))
    print(sql)
    db_cursor.execute(sql)
    db.commit()

sql = "insert into DirectoryEntity(dname,parent_id) value {};".format(
    ("book", 1))
print(sql)
db_cursor.execute(sql)
db.commit()
sql = "insert into DirectoryEntity(dname,parent_id) value {};".format(
    ("test", 4))
print(sql)
db_cursor.execute(sql)
db.commit()
sql = "insert into DirectoryEntity(dname,parent_id) value {};".format(
    ("test", 2))
print(sql)
db_cursor.execute(sql)
db.commit()

if os.path.exists(pool_path):
    os.system("rm -rf "+pool_path)
os.mkdir(pool_path)

users = ["test", "test1", "test2"]
passwds = ["test", "test1", "test2"]
for j in range(1, 4):
    sql = "insert into UserEntity(user_name,password_hash,root_dir_id) value {};".format(
        (users[j - 1], hashlib.md5(passwds[j - 1].encode("gbk")).hexdigest(),
         4 - j))
    print(sql)
    db_cursor.execute(sql)
    db.commit()

with open("test.pdf", "rb") as f:
    f_md5 = hashlib.md5(f.read()).hexdigest()
    fsize = f.tell()#当前文件指针位置
    sql = "insert into FileEntity(MD5,fsize,link_num,next_index,is_complete) value {}".\
        format((f_md5,fsize,2,-1,True))
    print(sql)
    db_cursor.execute(sql)
    db.commit()
    f.seek(0, 0)#回开头
    num = 1
    while fsize > f.tell():
        fgsize = min(fsize - f.tell(), 4 * 1024 * 1024)
        tmp = f.read(fgsize)
        fg_md5 = hashlib.md5(tmp).hexdigest()
        sql = "insert into FileFragmentEntity(MD5,fgsize,link_num) value {};".\
            format((fg_md5,fgsize,1))
        print(sql)
        db_cursor.execute(sql)
        db.commit()
        sql = "insert into FileFragmentMap(fid,`index`,fgid) value {};".\
            format((1,num-1,num))
        print(sql)
        db_cursor.execute(sql)
        db.commit()
        with open(pool_path+"/{}".format(fg_md5), "wb") as fp:
            fp.write(tmp)
        num += 1
    sql = "insert into FileDirectoryMap(fid,did,fname) value {}".\
        format((1,6,"test.pdf"))
    print(sql)
    db_cursor.execute(sql)
    db.commit()
    sql = "insert into FileDirectoryMap(fid,did,fname) value {}".\
        format((1,5,"book.pdf"))
    print(sql)
    db_cursor.execute(sql)
    db.commit()
