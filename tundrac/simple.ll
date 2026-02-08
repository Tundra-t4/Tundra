; ModuleID = 'simple.ta'
source_filename = "simple.ta"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-darwin25.0.0"

@tmp4a = internal global i32 0
@tmp5 = private unnamed_addr constant [13 x i8] c"Hello world\0A\00", align 1
@tmp4b = internal global ptr null
@tmp6 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@tmp4c = internal global i1 false
@tmp15 = private unnamed_addr constant [7 x i8] c"True!\0A\00", align 1
@tmp17 = private unnamed_addr constant [8 x i8] c"False!\0A\00", align 1
@tmp27 = private unnamed_addr constant [9 x i8] c"A is %d\0A\00", align 1
@tmp34 = private unnamed_addr constant [43 x i8] c"double it and give it to the next one: %d\0A\00", align 1

define internal i32 @mainTundraStart() {
__allocreigonmainTundraStart:
  %tmp19a = alloca i32, align 4
  %flagtmp19a = alloca i1, align 1
  store i1 false, ptr %flagtmp19a, align 1
  %retvalmainTundraStart = alloca i32, align 4
  br label %tmp2

tmp2:                                             ; preds = %__allocreigonmainTundraStart
  br label %tmp3

tmp3:                                             ; preds = %tmp2
  br label %tmp4

tmp4:                                             ; preds = %tmp3
  store i32 1, ptr @tmp4a, align 4
  store ptr @tmp5, ptr @tmp4b, align 8
  %tmp7 = load i32, ptr @tmp4a, align 4
  %tmp8 = call i32 (ptr, ...) @printf(ptr @tmp6, i32 %tmp7)
  store i1 true, ptr @tmp4c, align 1
  %tmp13 = load i1, ptr @tmp4c, align 1
  %tmp14 = icmp eq i1 %tmp13, true
  br i1 %tmp14, label %tmp10, label %tmp11

tmp10:                                            ; preds = %tmp4
  %tmp16 = call i32 (ptr, ...) @printf(ptr @tmp15)
  br label %tmp12

tmp11:                                            ; preds = %tmp4
  %tmp18 = call i32 (ptr, ...) @printf(ptr @tmp17)
  br label %tmp12

tmp12:                                            ; preds = %tmp11, %tmp10
  %tmp22 = load i32, ptr @tmp4a, align 4
  %tmp23 = icmp ne i32 %tmp22, 20
  br i1 %tmp23, label %tmp19, label %tmp20

tmp19:                                            ; preds = %tmp19, %tmp12
  %tmp24 = load i32, ptr @tmp4a, align 4
  %0 = load i1, ptr %flagtmp19a, align 1
  %1 = icmp eq i1 %0, true
  %2 = select i1 %1, ptr %tmp19a, ptr @tmp4a
  %tmp241 = load i32, ptr %tmp19a, align 4
  %3 = select i1 %1, i32 %tmp241, i32 %tmp24
  store i32 %3, ptr %tmp19a, align 4
  store i1 true, ptr %flagtmp19a, align 1
  %tmp25 = load i32, ptr %2, align 4
  %tmp26 = add i32 %tmp25, 1
  store i32 %tmp26, ptr %tmp19a, align 4
  store i1 true, ptr %flagtmp19a, align 1
  %tmp28 = load i32, ptr %tmp19a, align 4
  %tmp29 = call i32 (ptr, ...) @printf(ptr @tmp27, i32 %tmp28)
  %tmp30 = load i32, ptr %tmp19a, align 4
  %tmp31 = load i32, ptr %tmp19a, align 4
  %tmp32 = icmp ne i32 %tmp31, 20
  br i1 %tmp32, label %tmp19, label %tmp20

tmp20:                                            ; preds = %tmp19, %tmp12
  call void @my_function(i32 67890)
  store i32 0, ptr %retvalmainTundraStart, align 4
  store i32 0, ptr %retvalmainTundraStart, align 4
  %tmp37 = load i32, ptr %retvalmainTundraStart, align 4
  ret i32 %tmp37
}

define i32 @main() {
tmp0:
  %tmp1 = call i32 @mainTundraStart()
  ret i32 %tmp1
}

declare i32 @printf(ptr, ...)

define internal void @my_function(i32 %0) {
__allocreigonmy_function:
  %tmp33a = alloca i32, align 4
  br label %tmp33

tmp33:                                            ; preds = %__allocreigonmy_function
  store i32 %0, ptr %tmp33a, align 4
  %tmp35 = load i32, ptr %tmp33a, align 4
  %tmp36 = call i32 (ptr, ...) @printf(ptr @tmp34, i32 %tmp35)
  ret void
}
