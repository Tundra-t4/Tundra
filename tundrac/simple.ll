; ModuleID = 'simple.ta'
source_filename = "simple.ta"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-darwin25.0.0"

%none_struct = type opaque

@none = internal global %none_struct zeroinitializer
@noneload = internal constant ptr @none
@tmp4 = private unnamed_addr constant [15 x i8] c"example string\00", align 1
@tmp44 = private unnamed_addr constant [15 x i8] c"GenericPointer\00", align 1
@tmp74 = private unnamed_addr constant [11 x i8] c"Map Failed\00", align 1
@tmp61test = internal global ptr null

define internal i32 @mainTundraStart() {
__allocreigonmainTundraStart:
  %tmp36tmp37 = alloca ptr, align 8
  %tmp36tmp37_tundra_ptr_alloc = alloca i8, align 1
  %tmp41tmp42 = alloca ptr, align 8
  %tmp41tmp42_tundra_ptr_alloc = alloca i8, align 1
  %tmp41tmp43 = alloca ptr, align 8
  %tmp41tmp43_tundra_ptr_alloc = alloca i8, align 1
  %tmp41tmp45 = alloca ptr, align 8
  %tmp41tmp45_tundra_ptr_alloc = alloca i8, align 1
  %tmp41tmp46 = alloca ptr, align 8
  %tmp41tmp46_tundra_ptr_alloc = alloca i8, align 1
  %tmp41tmp47 = alloca ptr, align 8
  %tmp41tmp47_tundra_ptr_alloc = alloca i8, align 1
  %tmp41tmp48 = alloca ptr, align 8
  %tmp41tmp48_tundra_ptr_alloc = alloca i8, align 1
  %tmp41tmp49 = alloca ptr, align 8
  %tmp41tmp49_tundra_ptr_alloc = alloca i8, align 1
  %tmp41tmp50 = alloca ptr, align 8
  %tmp41tmp50_tundra_ptr_alloc = alloca i8, align 1
  %tmp41tmp51 = alloca ptr, align 8
  %tmp41tmp51_tundra_ptr_alloc = alloca i8, align 1
  %tmp41tmp52 = alloca ptr, align 8
  %tmp41tmp52_tundra_ptr_alloc = alloca i8, align 1
  %tmp41tmp53 = alloca ptr, align 8
  %tmp41tmp53_tundra_ptr_alloc = alloca i8, align 1
  %tmp41tmp54 = alloca ptr, align 8
  %tmp41tmp54_tundra_ptr_alloc = alloca i8, align 1
  %tmp41tmp55 = alloca ptr, align 8
  %tmp41tmp55_tundra_ptr_alloc = alloca i8, align 1
  %tmp41tmp56 = alloca ptr, align 8
  %tmp41tmp56_tundra_ptr_alloc = alloca i8, align 1
  %tmp41tmp57 = alloca ptr, align 8
  %tmp41tmp57_tundra_ptr_alloc = alloca i8, align 1
  %tmp41tmp58 = alloca ptr, align 8
  %tmp41tmp58_tundra_ptr_alloc = alloca i8, align 1
  %tmp41tmp59 = alloca ptr, align 8
  %tmp41tmp59_tundra_ptr_alloc = alloca i8, align 1
  %tmp41tmp60 = alloca ptr, align 8
  %tmp41tmp60_tundra_ptr_alloc = alloca i8, align 1
  %tmp61tmp62 = alloca ptr, align 8
  %tmp61tmp62_tundra_ptr_alloc = alloca i8, align 1
  %retvalmainTundraStart = alloca i32, align 4
  br label %tmp2

tmp2:                                             ; preds = %__allocreigonmainTundraStart
  br label %tmp3

tmp3:                                             ; preds = %tmp2
  br label %tmp5

tmp5:                                             ; preds = %tmp3
  br label %tmp6

tmp6:                                             ; preds = %tmp5
  br label %tmp7

tmp7:                                             ; preds = %tmp6
  br label %tmp8

tmp8:                                             ; preds = %tmp7
  br label %tmp9

tmp9:                                             ; preds = %tmp8
  br label %tmp10

tmp10:                                            ; preds = %tmp9
  br label %tmp13

tmp13:                                            ; preds = %tmp10
  br label %tmp14

tmp14:                                            ; preds = %tmp13
  br label %tmp15

tmp15:                                            ; preds = %tmp14
  br label %tmp16

tmp16:                                            ; preds = %tmp15
  br label %tmp17

tmp17:                                            ; preds = %tmp16
  br label %tmp18

tmp18:                                            ; preds = %tmp17
  br label %tmp19

tmp19:                                            ; preds = %tmp18
  br label %tmp20

tmp20:                                            ; preds = %tmp19
  br label %tmp21

tmp21:                                            ; preds = %tmp20
  br label %tmp22

tmp22:                                            ; preds = %tmp21
  br label %tmp23

tmp23:                                            ; preds = %tmp22
  br label %tmp24

tmp24:                                            ; preds = %tmp23
  br label %tmp25

tmp25:                                            ; preds = %tmp24
  br label %tmp26

tmp26:                                            ; preds = %tmp25
  br label %tmp27

tmp27:                                            ; preds = %tmp26
  br label %tmp28

tmp28:                                            ; preds = %tmp27
  br label %tmp29

tmp29:                                            ; preds = %tmp28
  br label %tmp30

tmp30:                                            ; preds = %tmp29
  br label %tmp31

tmp31:                                            ; preds = %tmp30
  br label %tmp32

tmp32:                                            ; preds = %tmp31
  br label %tmp33

tmp33:                                            ; preds = %tmp32
  br label %tmp34

tmp34:                                            ; preds = %tmp33
  br label %tmp35

tmp35:                                            ; preds = %tmp34
  br label %tmp36

tmp36:                                            ; preds = %tmp35
  store i8 0, ptr %tmp36tmp37_tundra_ptr_alloc, align 1
  store ptr %tmp36tmp37_tundra_ptr_alloc, ptr %tmp36tmp37, align 8
  br label %tmp38

tmp38:                                            ; preds = %tmp36
  br label %tmp39

tmp39:                                            ; preds = %tmp38
  br label %tmp40

tmp40:                                            ; preds = %tmp39
  br label %tmp41

tmp41:                                            ; preds = %tmp40
  store i8 0, ptr %tmp41tmp42_tundra_ptr_alloc, align 1
  store ptr %tmp41tmp42_tundra_ptr_alloc, ptr %tmp41tmp42, align 8
  store i8 0, ptr %tmp41tmp43_tundra_ptr_alloc, align 1
  store ptr %tmp41tmp43_tundra_ptr_alloc, ptr %tmp41tmp43, align 8
  store i8 65, ptr %tmp41tmp45_tundra_ptr_alloc, align 1
  store ptr %tmp41tmp45_tundra_ptr_alloc, ptr %tmp41tmp45, align 8
  store i8 65, ptr %tmp41tmp46_tundra_ptr_alloc, align 1
  store ptr %tmp41tmp46_tundra_ptr_alloc, ptr %tmp41tmp46, align 8
  store i8 65, ptr %tmp41tmp47_tundra_ptr_alloc, align 1
  store ptr %tmp41tmp47_tundra_ptr_alloc, ptr %tmp41tmp47, align 8
  store i8 65, ptr %tmp41tmp48_tundra_ptr_alloc, align 1
  store ptr %tmp41tmp48_tundra_ptr_alloc, ptr %tmp41tmp48, align 8
  store i8 65, ptr %tmp41tmp49_tundra_ptr_alloc, align 1
  store ptr %tmp41tmp49_tundra_ptr_alloc, ptr %tmp41tmp49, align 8
  store i8 65, ptr %tmp41tmp50_tundra_ptr_alloc, align 1
  store ptr %tmp41tmp50_tundra_ptr_alloc, ptr %tmp41tmp50, align 8
  store i8 0, ptr %tmp41tmp51_tundra_ptr_alloc, align 1
  store ptr %tmp41tmp51_tundra_ptr_alloc, ptr %tmp41tmp51, align 8
  store i8 0, ptr %tmp41tmp52_tundra_ptr_alloc, align 1
  store ptr %tmp41tmp52_tundra_ptr_alloc, ptr %tmp41tmp52, align 8
  store i8 0, ptr %tmp41tmp53_tundra_ptr_alloc, align 1
  store ptr %tmp41tmp53_tundra_ptr_alloc, ptr %tmp41tmp53, align 8
  store i8 0, ptr %tmp41tmp54_tundra_ptr_alloc, align 1
  store ptr %tmp41tmp54_tundra_ptr_alloc, ptr %tmp41tmp54, align 8
  store i8 0, ptr %tmp41tmp55_tundra_ptr_alloc, align 1
  store ptr %tmp41tmp55_tundra_ptr_alloc, ptr %tmp41tmp55, align 8
  store i8 0, ptr %tmp41tmp56_tundra_ptr_alloc, align 1
  store ptr %tmp41tmp56_tundra_ptr_alloc, ptr %tmp41tmp56, align 8
  store i8 0, ptr %tmp41tmp57_tundra_ptr_alloc, align 1
  store ptr %tmp41tmp57_tundra_ptr_alloc, ptr %tmp41tmp57, align 8
  store i8 0, ptr %tmp41tmp58_tundra_ptr_alloc, align 1
  store ptr %tmp41tmp58_tundra_ptr_alloc, ptr %tmp41tmp58, align 8
  store i8 0, ptr %tmp41tmp59_tundra_ptr_alloc, align 1
  store ptr %tmp41tmp59_tundra_ptr_alloc, ptr %tmp41tmp59, align 8
  store i8 0, ptr %tmp41tmp60_tundra_ptr_alloc, align 1
  store ptr %tmp41tmp60_tundra_ptr_alloc, ptr %tmp41tmp60, align 8
  br label %tmp61

tmp61:                                            ; preds = %tmp41
  store i8 0, ptr %tmp61tmp62_tundra_ptr_alloc, align 1
  store ptr %tmp61tmp62_tundra_ptr_alloc, ptr %tmp61tmp62, align 8
  %tmp101 = call ptr @allocate_guarded(i64 8192)
  store ptr %tmp101, ptr @tmp61test, align 8
  store i32 0, ptr %retvalmainTundraStart, align 4
  store i32 0, ptr %retvalmainTundraStart, align 4
  %tmp102 = load i32, ptr %retvalmainTundraStart, align 4
  ret i32 %tmp102
}

define i32 @main() {
tmp0:
  %tmp1 = call i32 @mainTundraStart()
  ret i32 %tmp1
}

declare i32 @"1178946130363361532print"(ptr)

define internal ptr @anonfn0() {
__allocreigonanonfn0:
  %retvalanonfn0 = alloca ptr, align 8
  br label %tmp11

tmp11:                                            ; preds = %__allocreigonanonfn0
  store ptr @tmp4, ptr %retvalanonfn0, align 8
  %tmp12 = load ptr, ptr %retvalanonfn0, align 8
  ret ptr %tmp12
}

declare ptr @"1178946130363361532AssertDefer"(i1, ptr)

declare i32 @"1178946130363361532println"(ptr)

declare ptr @"1178946130363361532Abort"(ptr)

declare ptr @"1178946130363361532Assert"(i1, ptr)

declare ptr @"1178946130363361532SubPanic"(ptr, i64)

declare i64 @"1178946130363361532Address"(ptr)

declare i32 @printf(ptr, ...)

declare i8 @getchar()

declare ptr @malloc(i64)

declare ptr @realloc(ptr, i64)

declare ptr @strcpy(ptr, ptr)

declare ptr @strcat(ptr, ptr)

declare i32 @free(ptr)

declare i32 @exit(i32)

declare i32 @sprintf(ptr, ptr, ...)

declare i32 @system(ptr)

declare i32 @strlen(ptr)

declare i32 @sleep(i32)

declare i64 @ptrace(i32, i64, ptr, ptr)

declare i64 @fork()

declare ptr @memcpy(ptr, ptr, i64)

declare i32 @vm_protect(i64, ptr, i64, i32, i32)

declare i64 @mach_task_self()

declare ptr @mmap(i64, i64, i32, i32, i32, i32)

declare ptr @memset(ptr, i32, i64)

define internal ptr @allocate_guarded(i64 %0) {
__allocreigonallocate_guarded:
  %tmp63size = alloca i64, align 8
  %tmp63pages = alloca i64, align 8
  %tmp63ptr = alloca ptr, align 8
  %tmp63ptr_addr = alloca i64, align 8
  %tmp71addr = alloca i64, align 8
  %tmp71pages = alloca i64, align 8
  %tmp82addr = alloca i64, align 8
  %flagtmp82addr = alloca i1, align 1
  store i1 false, ptr %flagtmp82addr, align 1
  %tmp82ptr = alloca ptr, align 8
  %flagtmp82ptr = alloca i1, align 1
  store i1 false, ptr %flagtmp82ptr, align 1
  %tmp82pages = alloca i64, align 8
  %flagtmp82pages = alloca i1, align 1
  store i1 false, ptr %flagtmp82pages, align 1
  %tmp71tmp84 = alloca ptr, align 8
  %flagtmp71tmp84 = alloca i1, align 1
  store i1 false, ptr %flagtmp71tmp84, align 1
  %tmp71ptr = alloca ptr, align 8
  %retvalallocate_guarded = alloca ptr, align 8
  br label %tmp63

tmp63:                                            ; preds = %__allocreigonallocate_guarded
  store i64 %0, ptr %tmp63size, align 8
  %tmp64 = load i64, ptr %tmp63size, align 8
  %tmp65 = sdiv i64 %tmp64, 4096
  store i64 %tmp65, ptr %tmp63pages, align 8
  %tmp66 = call ptr @mmap(i64 0, i64 4096, i32 3, i32 4098, i32 -1, i32 0)
  store ptr %tmp66, ptr %tmp63ptr, align 8
  %tmp67 = load ptr, ptr %tmp63ptr, align 8
  %tmp68 = ptrtoint ptr %tmp67 to i64
  store i64 %tmp68, ptr %tmp63ptr_addr, align 8
  %tmp72 = load i64, ptr %tmp63ptr_addr, align 8
  %tmp73 = icmp eq i64 %tmp72, -1
  br i1 %tmp73, label %tmp70, label %tmp71

tmp70:                                            ; preds = %tmp63
  %tmp75 = call ptr @"1178946130363361532Abort"(ptr @tmp74)
  br label %tmp71

tmp71:                                            ; preds = %tmp70, %tmp63
  %tmp76 = load ptr, ptr %tmp63ptr, align 8
  %tmp77 = ptrtoint ptr %tmp76 to i64
  store i64 %tmp77, ptr %tmp71addr, align 8
  %tmp78 = load i64, ptr %tmp71addr, align 8
  %tmp79 = add i64 %tmp78, 4096
  store i64 %tmp79, ptr %tmp71addr, align 8
  %tmp80 = load i64, ptr %tmp63pages, align 8
  %tmp81 = sub i64 %tmp80, 1
  store i64 %tmp81, ptr %tmp71pages, align 8
  %tmp85 = load i64, ptr %tmp71pages, align 8
  %tmp86 = icmp ne i64 %tmp85, 0
  br i1 %tmp86, label %tmp82, label %tmp83

tmp82:                                            ; preds = %tmp82, %tmp71
  %tmp87 = load i64, ptr %tmp71addr, align 8
  %1 = load i1, ptr %flagtmp82addr, align 1
  %2 = icmp eq i1 %1, true
  %3 = select i1 %2, ptr %tmp82addr, ptr %tmp71addr
  %tmp871 = load i64, ptr %tmp82addr, align 8
  %4 = select i1 %2, i64 %tmp871, i64 %tmp87
  store i64 %4, ptr %tmp82addr, align 8
  store i1 true, ptr %flagtmp82addr, align 1
  %tmp88 = load i64, ptr %3, align 8
  %tmp89 = call ptr @mmap(i64 %tmp88, i64 4096, i32 3, i32 4098, i32 -1, i32 0)
  store ptr %tmp89, ptr %tmp82ptr, align 8
  store i1 true, ptr %flagtmp82ptr, align 1
  %tmp90 = load i64, ptr %tmp82addr, align 8
  %tmp91 = add i64 %tmp90, 4096
  store i64 %tmp91, ptr %tmp82addr, align 8
  store i1 true, ptr %flagtmp82addr, align 1
  %tmp92 = load i64, ptr %tmp71pages, align 8
  %5 = load i1, ptr %flagtmp82pages, align 1
  %6 = icmp eq i1 %5, true
  %7 = select i1 %6, ptr %tmp82pages, ptr %tmp71pages
  %tmp922 = load i64, ptr %tmp82pages, align 8
  %8 = select i1 %6, i64 %tmp922, i64 %tmp92
  store i64 %8, ptr %tmp82pages, align 8
  store i1 true, ptr %flagtmp82pages, align 1
  %tmp93 = load i64, ptr %7, align 8
  %tmp94 = sub i64 %tmp93, 1
  store i64 %tmp94, ptr %tmp82pages, align 8
  store i1 true, ptr %flagtmp82pages, align 1
  %tmp95 = load ptr, ptr %tmp82ptr, align 8
  store ptr %tmp95, ptr %tmp71tmp84, align 8
  store i1 true, ptr %flagtmp71tmp84, align 1
  %tmp96 = load i64, ptr %tmp82pages, align 8
  %tmp97 = icmp ne i64 %tmp96, 0
  br i1 %tmp97, label %tmp82, label %tmp83

tmp83:                                            ; preds = %tmp82, %tmp71
  %tmp98 = load ptr, ptr %tmp71tmp84, align 8
  store ptr %tmp98, ptr %tmp71ptr, align 8
  %tmp99 = load ptr, ptr %tmp71ptr, align 8
  store ptr %tmp99, ptr %retvalallocate_guarded, align 8
  %tmp100 = load ptr, ptr %retvalallocate_guarded, align 8
  ret ptr %tmp100
}
