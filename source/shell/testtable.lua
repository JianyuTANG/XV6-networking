a={2,4,5,3,1}
b={4,-1,2.3,5,7}
table.move(b,1,5,6,a)
table.insert(a,2,233)
print(table.concat(a,','))
table.remove(a,3)
table.sort(a)
print(table.concat(a,','))
