# OpenDb


# Write


# Read


# Compaction

##  Api

* level0_file_num_compaction_trigger: 当L0的个数超过这个值时，会触发compaction
* max_bytes_for_level_base: L1的SST文件总大大小阈值，L2层SST文件总大小阈值为max_bytes_for_level_base*ratio, L3层，L4层，以此类推
* target_file_size_multiplier: 下一层单个SST文件是上一层单个SST文件大小的几倍
* max_bytes_for_level_multipliers: 下一层SST文件的总大小最大值是当前层SST文件总大小最大值的几倍
* num_levels: LSM树最多有多少层

## Doc
  
 https://zhuanlan.zhihu.com/p/369386111