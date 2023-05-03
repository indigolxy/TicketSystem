# UserSystem
## user信息：
**username**, password, name, mailAddr, privilege, **logged_in**
### username
由字母开头，由字母、数字和下划线组成的字符串，长度不超过 20。
### password
由可见字符组成的字符串，长度不低于 1，不超过 30。
### name
由 2 至 5 个汉字组成的字符串。
### mailAddr
仅含数字、大小写字母，@ 和 .，长度不超过 30（无需检验邮箱格式的合法性）。
### privilege
0~10 中的一个整数。
### logged_in
bool
## 需要的文件
一个所有用户的，从username映射到userInfo的bpt（key值不重复，正常的树）
## 操作
### add_user(N)
查找当前用户是否已经登录及其权限
查找需要添加的username，如果存在就失败，否则向bpt中添加这一新用户的索引，并在用户信息文件中添加一条记录
### login(F)
查找用户是否存在，密码是否正确（第一个bpt）
查找用户是否已经登录，若无，在bpt中添加这一用户，登录成功（第二个bpt）
### logout(F)
查找用户是否已经登录，若有，在bpt中删除这一用户，登出成功（第二个bpt）
### query_profile(SF)
查找当前用户是否已经登录以及当前用户的权限（第二个bpt）
查找查询用户的信息和权限（第一个bpt）
### modify_profile(F)
查找当前用户是否已经登录以及当前用户的权限（第二个bpt）
查找查询用户的信息和权限（第一个bpt）
如果用户存在，修改查询用户的信息（user文件）

# Order
## order信息
username, trainID, from, to, leavingtime, arrivingtime, price, num
**status**

# Train
## train信息
**trainID**, stationNum, stations, seatNum, **seats**（初始等于seatNum）
prices, startTime, travelTimes, stopoverTimes, saleDate, type
## 文件
一个存放所有火车信息的文件
一个所有train的，从trainID映射到火车信息文件位置的bpt（正常bpt）
一个已发布的train的，从trainID映射到火车信息文件位置的bpt（正常bpt）
## 操作
### add_train(N)
查找trainID是否存在（bpt）
若不存在，向bpt中添加trainID，并在train文件中添加一条记录
（很显然add的时候未发布，故不需要修改第二个bpt）
### delete_train(N)
查找第一个、第二个bpt
若第一个bpt中有trainID，第二个bpt没有，则在第一个bpt中删除，并在train文件中删除
### release_train(N)
在第一个bpt中查找，若存在，则添加到第二个bpt中
### query_train(N)
去第一个bpt里查找trainID
去train文件里查找信息并处理后输出
### query_ticket(SF)


# 注意事项
1. bpt中“先查找是否存在，若不存在添加/若存在删除”的操作可通过调整bpt的接口，只在bpt中查找一次，不用另外调用一次find
2. 对于正常的bpt可以提供一个正常的find接口