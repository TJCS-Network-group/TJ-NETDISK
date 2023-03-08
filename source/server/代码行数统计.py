import os
def CodeLinesCount(path:str)->int:
    ans=0
    files=os.listdir(path)
    for file in files:
        newpath=path+'/'+file
        if not os.path.isdir(newpath):
            f=open(newpath)
            ans+=len(f.readlines())
        else:
            ans=ans+CodeLinesCount(newpath)
    return ans
path="."
print(CodeLinesCount(path))