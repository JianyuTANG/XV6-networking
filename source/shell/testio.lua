print("a=io.open('testio.txt','w+')")
a=io.open('testio.txt','w+')
print("a:write('123')")
a:write('123')
print("a:seek('set',1)")
a:seek('set',1)
print("a:write('4')")
a:write('4')
print("a:close()")
a:close()
print("os.rename('testio.txt','renamed.txt')")
os.rename('testio.txt','renamed.txt')
print("b=io.open('renamed.txt','r')")
b=io.open('renamed.txt','r')
print("print(b:read())")
print(b:read())
print("b:close()")
b:close()
print("os.remove('renamed.txt')")
os.remove('renamed.txt')
