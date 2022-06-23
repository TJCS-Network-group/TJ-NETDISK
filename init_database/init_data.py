#!/bin/bash python3
# -*- coding: gbk -*-
from copy import deepcopy
import os
import hashlib
import pymysql as pm
db = pm.connect(host='localhost',user='root',password='root123',database='pan',charset="gbk")


db_cursor = db.cursor()
with open("pan.sql","r",encoding='gbk') as f:
    x = list(map(lambda x:x.strip(),f.readlines()))
    t = list()
    p = ""
    for i in x:
        p += deepcopy(i)
        if p[-1] == ";":
            t.append(p)
            p=""
    for i in t:
        db_cursor.execute(i)
        db.commit()
for j in range(1,4):
    sql = "insert into DirectoryEntity(id,dname,parent_id) value {};".format((j,"root",j))
    db_cursor.execute(sql)
    db.commit()

sql = "insert into DirectoryEntity(dname,parent_id) value {};".format(("book",1))
db_cursor.execute(sql)
db.commit()
sql = "insert into DirectoryEntity(dname,parent_id) value {};".format(("test",4))
db_cursor.execute(sql)
db.commit()
sql = "insert into DirectoryEntity(dname,parent_id) value {};".format(("test",2))
db_cursor.execute(sql)
db.commit()

users=["程森","高曾谊","陈冠忠"]
passwds=["1234567","7654321","1919810"]
for j in range(1,4):
    sql = "insert into UserEntity(user_name,password_hash,root_dir_id) value {};".format((users[j-1],hashlib.md5(passwds[j-1].encode("gbk")).hexdigest(),4-j))
    db_cursor.execute(sql)
    db.commit()



with open("test.pdf","rb") as f:
    f_md5 = hashlib.md5(f.read()).hexdigest()
    fsize = f.tell()
    sql = "insert into FileEntity(MD5,fsize,link_num,next_index,is_complete) value {}".\
        format((f_md5,fsize,2,-1,True))
    db_cursor.execute(sql)
    db.commit()
    f.seek(0,0)
    num = 1
    while fsize>f.tell():
        fgsize = min(fsize-f.tell(),4*1024*1024)
        tmp = f.read(fgsize)
        fg_md5 = hashlib.md5(tmp).hexdigest()
        sql = "insert into FileFragmentEntity(MD5,fgsize,link_num) value {};".\
            format((fg_md5,fgsize,1))

        db_cursor.execute(sql)
        db.commit()
        sql = "insert into FileFragmentMap(fid,`index`,fgid) value {};".\
            format((1,num,num))
        print(sql)
        db_cursor.execute(sql)
        db.commit()
        with open("../pool/{}".format(fg_md5),"wb") as fp:
            fp.write(tmp)
        num+=1
    sql = "insert into FileDirectoryMap(fid,did,fname) value {}".\
        format((1,6,"test.pdf"))
    db_cursor.execute(sql)
    db.commit()
    sql = "insert into FileDirectoryMap(fid,did,fname) value {}".\
        format((1,5,"book.pdf"))
    db_cursor.execute(sql)
    db.commit()


