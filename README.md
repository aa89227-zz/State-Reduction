# State-Reduction
利用遞歸式的查找，並嘗試將可合併的狀態進行合併，將狀態轉移圖最小化。

## 如何運行程式
編譯後執行，輸入資料檔名(.kiss file)

## 資料格式
### 輸入
.kiss file
```
# comment
.i <num-inputs>
.o <num-outputs>
.s <num-states>
.p <num-terms>
.r <reset-state>
<input> <current-state> <next-state> <output>
…
<input> <current-state> <next-state> <output>
.e
```

#### 輸入範例
```
# comment   "#"後為註解
.i 2        # input 為 2-digit
.o 1        # output 為 1-digit
.s 6        # 總共6個 state (0-5)
.p 11       # 總共11筆資料
.r s0       # 初始state
00 s0 s1 0  # state: s0 輸入: 00 -> 轉移到 s1 輸出: 0
01 s0 s2 1  # state: s0 輸入: 01 -> 轉移到 s2 輸出: 1
10 s0 s3 1  # state: s0 輸入: 10 -> 轉移到 s3 輸出: 1
11 s1 s2 0
01 s1 s5 0
10 s2 s4 0
00 s2 s3 0
01 s3 s4 1
11 s3 s5 1
01 s4 s5 0
11 s5 s0 0
.e          # 終止輸入
            # state: s0 輸入: 11 -> 不理會
```

### 輸出
.kiss file
為最佳化的狀態圖
