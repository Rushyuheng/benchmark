# Benchmark Report

> Author INFO：
> F74076019
> 黃宇衡
> 資工111

## Develope Environment (Virtual Machine)
* OS : Ubuntu 18.04.2 LTS
* CPU : Intel(R) Core(TM) i7-9700 CPU @ 3.00GHz * 4
* System Memory : 1G
* HardDisk capacity：80G
* Programming Landuage : C++ 
* Compiler : g++ (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0
* VMware : Oracle Virtual Box 6.0.10 r132072 (Qt5.6.2)

## How to run my code
```shell=
# compile
g++ mergesort.cpp -o mergesort

# execution
./mergesort [memory size (unit:GB)] [input file path]
```
ex:
```shell =
g++ mergesort.cpp -o mergesort
./mergesoet 8 ./input.txt
```

## Program Developement & Runtime result
### 開發邏輯：
本次採取的作法是2-Way Mergesort搭配用vector和deque實作的I/O Buffer來達到大檔案排序
1. 將大檔案讀入程式中，每次讀到Buffer滿了後就將呼叫內建函式庫的sort()，Buffer必小於記憶體可以直接進行排序，排序後把Buffer的資料輸出。將大檔案切割成排序好的小檔案，每個檔案大小均不大於Buffer
2. 將切割後的小檔案倆倆讀入，使用無窮迴圈+eof()讀檔到input Buffer中，一次讀入一個inputBuffer的大小
3. 將兩個input buffer依照第一個值的大小merge到output Buffer裡，如果output Buffer滿了就將其資料輸出成子txt檔案
4. 依序讀入兩檔案直到其中一檔案抵達EOF且該檔案的input Buffer也清空了，代表另一個input Buffer和檔案中剩下的值均大於已清空的input檔案
5. 把剩下一個未處理完的檔案資料全部複製到子output file的結尾
6. 迴圈重複步驟2-5直到子檔案被merge到剩下一個檔案即為output
:::warning
我採用iterative的方式非recursive的方式去merge file，我認為recursive的stack堆疊會使得每次merge的資料大小上限變小， 因為recursive會儲存很多caller的狀態在stack裡等待直到被return回來才會釋出空間
:::

### 執行時間結果：
當測資為2.7G的txt:
![](https://i.imgur.com/i6h9vE0.png)


## Analysis Report 
### Program optimization
* 在2-way merge的過程中，開發前有考慮用n-heap來一次merge全部的檔案，但發現用n-heap就只能一次從file裡拿取一個資料，會花很多時間去做Disk I/O，後來改用vector當buffer的方式讀入資料，每次讀入的大小是記憶體的四分之一，使用buffer可以減少從ifstream讀取資料(Disk I/O)的次數
* 一開始我是將buffer的大小寫死在程式中，且buffer的大小就是記憶體大小，未考慮到程式中其他的變數也佔記憶體，導致光執行一隻程式就會觸發系統swap的機制
![](https://i.imgur.com/QnDpLtY.png)

    而swap本身是在磁碟上讀取，會非常慢更占用很多cpu的時間，因此我在後續的版本要求使用者執行前加入記憶體作為參數，而我就不將buffer配置到跟記憶體一樣的大小，減少執行一個程式就要swap的狀況，同時也能因應不同硬體環境決定buffer上限，避免有很多記憶體卻受限於hard code的限制，盡可能產生最少個merge的子檔案數量，
* 本程式在切割檔案時我是採用vector實作buffer，```vector.push_back()```本身雖然可以自己動態配置記憶體大小，但其配置方法為： 「每當要加入新元素時，如果記憶體大小不足，則將vector大小擴增至原本的兩倍」，merge前切割子檔案時已知每次檔案的上限，我就直接改用```vector.reserve()```一次把全部需要的空間配置完畢，就避免重複的消耗資源的malloc
[cplusplus push_back()](http://www.cplusplus.com/reference/vector/vector/push_back/)
 
* 後續查資料時發現我原先習慣在寫入檔案時用endl來做換行，但endl會同時清空ostream中的output buffer，於是改採用"\n"
[資料來源](https://stackoverflow.com/questions/213907/stdendl-vs-n)
[cplusplus std::endl](https://www.cplusplus.com/reference/ostream/endl)

* 原先在merge檔案階段也是使用vector作為buffer，其中會將vector的第一個元素記錄下來再erase掉，但後來查資料以及實驗發現，vector若是erase中間的資料，會搬動全部後面資料的記憶體位置，導致非常沒有效率，實驗跑了將近10小時還未完成(如附圖)，後來選用deque，這樣的資料結構對開頭的第一個元素操作就不會頻繁的搬動其他資料，對程式的效率加速很多
[efficiency between vector.erase() v.s. deque.erase()](https://stackoverflow.com/questions/28266382/time-complexity-of-removing-items-in-vectors-and-deque)
![](https://i.imgur.com/ZzPQcLc.jpg)



### OS Performance Analysis
* 查看top發現本身我有四顆處理器均達到滿載(從Load average大於4中推得)，但同時執行2支mergesort時CPU的分配使用率卻非常不均衡，一支程式可以用到100%但另一支只有3.9%，作業系統並沒有讓四顆處理器平均分給兩支程式使用，而是全部先處理其中一個程式。
![](https://i.imgur.com/xf9vcid.png)
:::info
 我認為作業系統應該提供多程式平行分配給多個處理器處理的模式，才能讓多個程式的執行進度差不多
:::
* 另外也發現到當系統記憶體被占滿後，系統會用swap的方式處理超過記憶體的資料，但在swap的同時top中CPU的負載和各程式的使用率都降下來了，表示程式必須等待swap結束才能繼續進行，但若此時另一支程式中的資料都在記憶體內的話就應該能夠繼續執行才對
![](https://i.imgur.com/C0uucEm.png)
:::info
 我認為作業系統應該優化swap時被swap的資料保持在同一個程式使用範圍內，讓受swap影響的程式越少越好
:::
* 利用```perf trace -s ls -al```查看I/O狀態，可以看到電腦多次呼叫了```mmap```這個需要用到I/O的函式，而這個mmap是將需要Disk I/O轉的檔案映射到記憶體裡，這樣寫入磁碟就可以用寫入記憶體的方式執行，加速寫入，是linux系統內建一個加快I/O的方式
![](https://i.imgur.com/yC7Lwqr.png)
:::info
 一個好的作業系統應提供像mmap這樣將Disk I/O轉換成memory讀寫的函示，並主動調用，減少過多的Disk I/O
:::
[mmap 介紹](https://kknews.cc/zh-tw/news/4xyqyx2.html)
* 同時執行兩支mergesort，可以看到執行時間約為只執行一支的三倍(245sec -> 772sec)，linux的設計是搶占式多任務處理，但這次兩支程式的優先度是相同的，因此在搶占式多任務處理中分配到的時間是一樣的，也沒有一方可以中斷別人的執行，整能乖乖排隊，使得執行時間增加了很多
![](https://i.imgur.com/KaxCXyh.png)
:::info
 作業系統在設計multitasking時應考慮程式對各資源的使用程度來決定優先度，搶占式多任務處理是一個有效率的想法，但本次執行的程式是兩隻相同的程式，在這樣的規則下也沒獲得好處
:::