.data
msg_non_positive: .asciiz "Unable to check non-positive values \nExiting......"
msg_swap: .asciiz "End of range < start of range -- swapping values\n"
msg_start: .asciiz "Input the start of the range: "
msg_end: .asciiz "Input the end of the range:  "
output_space: .asciiz "\n"
output_start: .asciiz "Input the start of the range: "
output_end: .asciiz "Input the end of the range: "
output_and: .asciiz " and "
output_amicable: .asciiz " are amicable numbers \n"
output_range: .asciiz "Range of numbers: "
output_range_ch: .asciiz " - "
output_pairs: .asciiz "Pairs of amicable numbers = "

.text
main:

# print msg_start
    li      $v0,        4
    la      $a0,        msg_start
    syscall 

# scan start
    li      $v0,        5
    syscall 

# if start<=0, exit; else goto save_start
    bgt     $v0,        $zero,              save_start
    li      $v0,        4
    la      $a0,        msg_non_positive
    syscall 
    li      $v0,        10
    syscall 

save_start:

# save start to $s0
    add     $s0,        $v0,                $zero

# print msg_end
    li      $v0,        4
    la      $a0,        msg_end
    syscall 

# scan end
    li      $v0,        5
    syscall 

# if end <= 0, exit; else goto end_start
    bgt     $v0,        $zero,              save_end
    li      $v0,        4
    la      $a0,        msg_non_positive
    syscall 
    li      $v0,        10
    syscall 

save_end:

# save end to $s1
    add     $s1,        $v0,                $zero

# if start > end, swap
    ble     $s0,        $s1,                end_swap
    li      $v0,        4
    la      $a0,        msg_swap
    syscall 
    add     $t0,        $s1,                $zero
    add     $s1,        $s0,                $zero
    add     $s0,        $t0,                $zero

end_swap:

# save count to $s2
    add     $s2,        $zero,              $zero

# allocate array memory, array addrress $s3
    sub     $a0,        $s1,                $s0
    addi    $a0,        $a0,                1
    sll     $a0,        $a0,                2
    li      $v0,        9
    syscall 
    add     $s3,        $v0,                $zero

# calculate factor sum and save it to array
    add     $t0,        $s0,                $zero               # loop i
loop_1:
    li      $t1,        1                                       # loop j
    li      $t2,        0                                       # factorSum
    bgt     $t1,        $t0,                endLoop_2
loop_2:
    add     $a0,        $t1,                $zero
    add     $a1,        $t0,                $zero
    jal     isFactor
    nop     
    beq     $v0,        0,                  notFactor
    add     $t2,        $t2,                $t1
notFactor:
    addi    $t1,        $t1,                1
    blt     $t1,        $t0,                loop_2


endLoop_2:
    sub     $t3,        $t0,                $s0
    mul     $t3,        $t3,                4
    add     $t3,        $t3,                $s3
    sw      $t2,        0($t3)
    addi    $t0,        $t0,                1
    ble     $t0,        $s1,                loop_1



# look for a micable
    add     $t0,        $s0,                $zero               # loop i
loop_3:
    sub     $t3,        $t0,                $s0
    mul     $t3,        $t3,                4
    add     $t3,        $t3,                $s3
    lw      $t3,        0($t3)
    ble     $t3,        $t0,                endLoop_3
    addi    $t1,        $t0,                1                   # loop j
    bgt     $t1,        $s1,                endLoop_3

loop_4:
    sub     $t3,        $t1,                $s0
    mul     $t3,        $t3,                4
    add     $t3,        $t3,                $s3
    lw      $t3,        0($t3)
    bge     $t3,        $t1,                notAmicable

    add     $a0,        $t0,                $zero
    add     $a1,        $t1,                $zero
    jal     isAmicable
    nop     
    beq     $v0,        0,                  notAmicable

# print i
    li      $v0,        1
    add     $a0,        $t0,                $zero
    syscall 

# print
    li      $v0,        4
    la      $a0,        output_and
    syscall 


# print j
    li      $v0,        1
    add     $a0,        $t1,                $zero
    syscall 

# print
    li      $v0,        4
    la      $a0,        output_amicable
    syscall 

notAmicable:
    addi    $t1,        $t1,                1
    ble     $t1,        $s1,                loop_4


endLoop_3:
    addi    $t0,        $t0,                1
    ble     $t0,        $s1,                loop_3

# print
    li      $v0,        4
    la      $a0,        output_range
    syscall 

# print start
    li      $v0,        1
    add     $a0,        $s0,                $zero
    syscall 

# print
    li      $v0,        4
    la      $a0,        output_range_ch
    syscall 

# print end
    li      $v0,        1
    add     $a0,        $s1,                $zero
    syscall 

# print space
    li      $v0,        4
    la      $a0,        output_space
    syscall 

# print pair
    li      $v0,        4
    la      $a0,        output_pairs
    syscall 

# print count
    li      $v0,        1
    add     $a0,        $s2,                $zero
    syscall 


    li      $v0,        10
    syscall 


isFactor:
    addi    $sp,        $sp,                -12
    sw      $ra,        8($sp)
    sw      $a1,        4($sp)
    sw      $a0,        0($sp)
    div     $a1,        $a0
    li      $v0,        1
    mfhi    $t7
    beq     $t7,        0,                  factorReturn
    li      $v0,        0
factorReturn:
    addi    $sp,        $sp,                12
    jr      $ra


isAmicable:
    addi    $sp,        $sp,                -12
    sw      $ra,        8($sp)
    sw      $a1,        4($sp)
    sw      $a0,        0($sp)

    sub     $t7,        $a0,                $s0
    mul     $t7,        $t7,                4
    add     $t7,        $t7,                $s3
    lw      $t7,        0($t7)

    sub     $t6,        $a1,                $s0
    mul     $t6,        $t6,                4
    add     $t6,        $t6,                $s3
    lw      $t6,        0($t6)
    bne     $t7,        $a1,                amicableReturn0
    bne     $t6,        $a0,                amicableReturn0
    addi    $s2,        $s2,                1
    li      $v0,        1
    addi    $sp,        $sp,                12
    jr      $ra

amicableReturn0:
    li      $v0,        0
    addi    $sp,        $sp,                12
    jr      $ra