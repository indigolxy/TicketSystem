# UserSystem.h
## class
### UserInfo
username, password, name, mailAddr, privilege, logged_in, 
**order_num**(1-based)
#### username
由字母开头，由字母、数字和下划线组成的字符串，长度不超过 20。
#### password
由可见字符组成的字符串，长度不低于 1，不超过 30。
#### name
由 2 至 5 个汉字组成的字符串。
#### mailAddr
仅含数字、大小写字母，@ 和 .，长度不超过 30（无需检验邮箱格式的合法性）。
#### privilege
0~10 中的一个整数。
#### logged_in
bool
## files
一个所有用户的，从username映射到userInfo的bpt（key值不重复，正常的树）
## functions
### add_user(N) 
return: 成功/失败
查找当前用户是否已经登录及其权限
查找需要添加的username，如果存在就失败，否则向bpt中添加这一新用户的索引，并在用户信息文件中添加一条记录
### login(F)
return: 成功/失败
先查找用户是否存在，密码是否正确，是否已经登录
若未登录，修改bpt中value，更改其登录状态
### logout(F)
查找用户是否已经登录，若有，在bpt中删除这一用户，登出成功（第二个bpt）
### query_profile(SF)
查找当前用户是否已经登录以及当前用户的权限（第二个bpt）
查找查询用户的信息和权限（第一个bpt）
### modify_profile(F)
查找当前用户是否已经登录以及当前用户的权限（第二个bpt）
查找查询用户的信息和权限（第一个bpt）
如果用户存在，修改查询用户的信息（user文件）

# DateTime.h
## classes
### Date
int month, int date
### Time
int hour, int minute 
## functions
### IntToDate & DateToInt
int表示从6/1开始的第几天(6/1为0)
### IntToTime & TimeToInt
int表述从发车当天00:00开始的第几分钟(00:01为1)
IntToTime可能会>=24:00
### 重载输出

# TrainSystem.h
## classes
### TrainInfo
**trainID**, station_num(int), seat_num(int), released(bool), 
type(char), sale_date_start/end(int)
seats(Ptr), seats_waiting_lists(Ptr)（对应的文件中第一个(6/1)的SeatsDay/SeatsWaitingListsDay的位置）
以下均为大小为MAXStaionNum + 5的数组，下标统一
1. stations(10个汉字) *1 ~ station_num*
2. prices(int) *1 ~ station_num - 1*
3. arriving_times(int) *2 ~ station_num* 
4. leaving_times(int) *1 ~ station_num - 1* (leaving_times[1] == start_time)
### SeatsDay
int seats[StationNumMAX + 1]（对应每一站的seats）
`每一个trainID有92个SeatsDay，分别对应6/1到8/31，顺序存储在文件中`
### SeatsWaitingListDay
#### WaitingOrder
int start_station, int arrive_station, 
int num, char user_id[], int no

WaitingOrder waiting_orders[WaitingNumMAX]
int max_index（目前优先级最低的waiting_order的下标
### TrainStation
trainID, int index(在bpt中对应station在该车次中的下标)
### Ticket(只在内存中用，方便query_ticket和query_transfer时处理)
trainID, -s(char, leaving_time), -t(char, arriving_time), 
int seat, int time, int cost, int start_date
## files
一个从{station, trainID}映射到TrainStation的bpt(双键值)（released_train）
一个从trainID映射到trainInfo的bpt（正常bpt）（所有train）
一个文件(FileSystem)顺序存储所有train每一天的SeatsDay(92天，下标就是date的int形式)
一个文件(FileSystem)顺序存储所有train每一天的SeatsWaitingListDay
## functions
### add_train(N)
向第二个bpt中添加{trainID,trainInfo} (released = false)(两个Ptr设为-1(代表unreleased))
### delete_train(N)
查找第二个bpt，若不存在or released == true，删除失败
在第二个bpt中删除
### release_train(N)
查找第二个bpt，若存在且released == false则  
1. 把released改成true
2. 在第三个文件中写下对应的SeatsDay（均等于seat_num），并获取Ptr（修改TrainInfo）
3. 在第四个文件中写下空着的SeatsWaitingListDay，并获取Ptr
4. Modify回第二个bpt
5. 遍历车次的所有station并插入第一个bpt
### query_train(N)
去第二个bpt里查找，得到trainInfo，检查是否在该date当天有车次，若无，查询失败，int返回-1
如果released，去第三个文件中找到对应日期的SeatsDay，int返回1
如果unreleased，代表seats均等于seat_num，int返回0
return {{int, trainInfo}, SeatsDay}
### query_ticket(SF)
去第一个bpt里查找-s和-t对应的所有车次TrainStation(按trainID排序)（找不到返回空）
同时遍历两个vector，取交集，并
1. 比较一下TrainStation中index，t是不是在s后面，不符合则pass
2. 根据trainID进入第二个bpt查找出info，并检查date(处理成发车日期)是否在发布日期内，不是则pass
3. 定义一个Ticket，计算time，cost，并根据info中的Ptr去第三个文件中取出并计算出seat(取min)
4. 所有Ticket放入一个vector
按time/cost排序后返回该vector
### query_transfer(N) （必须恰好换乘一次）
去第一个bpt里查找-s和-t对应的所有车次TrainStation
分别遍历两个vector里的每一个车次的 -s后/-t前的 每一个station，
1. 先对-s对应vector里的每个train检查date(处理成发车日期)，下传TrainInfo
1. 找出可以换乘的pair{TrainStation, TrainStation}
2. 对找出的第二个TrainStation去第二个bpt查找TrainInfo
3. 检查换乘站的到达离开时间前后是否合理（先根据leave_date算出到达换乘站的时间日期，并据此算出，在该时间之后到达换乘站的，最早发车的arrive_train的发车日期）
4. 定义一个pair{Ticket, Ticket}，计算每个Ticket的time，cost，同时维护当前的最优解
5. 若当前的pair{Ticket, Ticket}更新了最优解，根据info中的Ptr去第三个文件中取出并计算出seat(取min)
6. 继续遍历检查pair{TrainStation, TrainStation}的所有stations，确保枚举全部的方案
返回最优解的pair{Ticket, Ticket}

其中对每一个pair{TrainStation, TrainStation}用一个CheckTrainStations来检查
此函数遍历枚举两个TrainStation之间所有可能的换乘方案，维护一个最优解，并返回{bool, pair{Ticket, Ticket}}最优解

再定义一个私有函数TicketPairCmp，用于判断两个pair<Ticket, Ticket>之间的大小关系（根据优先级）

# TicketSystem.h
## classes
### Order
trainID, date(转化为发车日期), num(int), price(int), status(int)(1 success，0 pending，-1 refunded), 
-s(int, char[], leaving_time), -t(int, char[], arriving_time)
seats_day(Ptr)（该车次**该发车日期**对应的SeatsDay的读写位置）, seats_waiting_list_day(Ptr)
## files
一个从{username, no}到Order的bpt（重复键值）
## 操作
### buy_ticket(SF)
1. 检查user已登录，并获取order_num, no = order_num + 1
2. 去TrainSystem里拿到trianID对应的TrainInfo
3. 检查train是否已经released，是否在当天有车次(失败return -1)
4. 定义一个Order对象，遍历trainInfo获取-s/-t对应的int（若不存在-s,-t,return -1)
5. 去TrainSystem里获取该车次该天的SeatsDay
6. 若剩余seats足够，status = 1，去TrianSystem里更改SeatsDay
7. 若剩余seats不够且不接受候补，购票失败，返回-1
8. 若剩余seats不够且接受候补，status = 0，定义一个WaitingOrder对象，去TrainSystem里更改SeatsWaitingListsDay
9. 向bpt中插入此order
10. user.order_num++，写回UserSystem(Modify)
10. 返回price/0(pending)/-1
### query_order(F)
1. 检查user已登录，并获取order_num
2. 去bpt里find所有username对应的order（按no升序）
3. 返回vector<Order>
### refund_ticket(N)
1. 检查user已登录，并获取order_num，计算出要找的no(check)
2. 去bpt里find{username, no}（单一键值）
3. 去TrainSystem里根据seats_day获取SeatsDay，并更改（完成退票）
4. order.status = -1，Modify bpt{username, no}
5. 去TrainSystem里根据seats_waiting_list_day获取SeatsWaitingList
6. 根据刚才更改好的SeatsDay顺序遍历整个SeatsWaitingList，找到可以补票的order时：
   更改SeatsDay完成补票，再根据{username, no}去bpt里把Order.status改为1(先find再Modify)，
   从SeatsWaitingList里去除已经补票的order
7. 把SeatsDay和SeatsWaitingList写回TrainSystem

# 有关bpt的使用
1. insert和remove函数会返回一个bool表示是否插入/删除成功（即是否存在）
2. find函数用于重复键值bpt
3. FindModify用于单一键值，单纯find或修改目标key对应的value（key不存在返回false）

# todo
1. 替换std::sort(去掉include)（适用于sjtu::vector的）
2. 更改UserSystem中order_num相关部分（接口）
3. 完成DateTimeTicketTrainInfoUserInfo等等的输出重载
4. 调整所有bpt的块长和缓存page的大小
5. 时间优化：CheckTrainStation中可以先比较tmp与ans，若tmp优于ans，再文件读写获取seats
6. 各system类的构造函数（涉及bpt的构造）
7. 指令类（输入的切片和输出的处理）

# 注意事项
1. 所有需要输出的，函数都只返回Info类或vector<Info>，在Info类重载输出，后续统一输出