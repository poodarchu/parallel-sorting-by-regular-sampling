# parallel sorting by regular sampling

PSRS(Parallel Sorting by Regular Sampling,并行正则采样排序)的基本原理如下：
    begin
        * 均匀划分：将n个元素A[1..n]均匀划分成p段，每个pi处理A[(i-1)n/p+1..in/p]
        * 局部排序：pi调用串行排序算法对A[(i-1)n/p+1..in/p]排序
        * 选取样本：pi从其有序子序列A[(i-1)n/p+1..in/p]中选取p个样本元素
        * 样本排序：用一台处理器对p2个样本元素进行串行排序
        * 选择主元：用一台处理器从排好序的样本序列中选取p-1个主元，并播送给其他pi
        * 主元划分： pi按主元将有序段A[(i-1)n/p+1..in/p]划分成p段
        * 全局交换：各处理器将其有序段按段号交换到对应的处理器中
        * 归并排序：各处理器对接收到的元素进行归并排序
    end.
实现三类并行:
1)基于OpenMP的并行；
2)基于MPI的并行；
3)基于MPI+OpenMP的并行。

数据集下载：链接: https://pan.baidu.com/s/1nvRZNO9 密码: jkmr
