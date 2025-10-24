.data
array: .space 4000004
msg_non_positive: .asciiz "Unable to check non-positive values \nExiting"
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

    la      $s3,        array


    li      $s4,        0                                   # loop i
    li      $s5,        0                                   # loop j
    li      $t0,        1                                   # min factor sum is 1
loop1:
    sll     $t1,        $s4,                2               # $t1 = i x 4
    add     $t2,        $s3,                $t1             # temp array[i] address $t0
    sw      $t0,        0($t2)
    addi    $s4,        $s4,                1
    ble     $s4,        $s1,                loop1

    li      $s4,        2

loop2:
    add     $s5,        $s4,                $s4
loop2_1:
    sll     $t1,        $s5,                2               # $t1 = j x 4
    add     $t0,        $s3,                $t1             # temp array[i] address $t0
    lw      $t2,        0($t0)
    add     $t2,        $t2,                $s4
    sw      $t2,        0($t0)
    add     $s5,        $s5,                $s4
    ble     $s5,        $s1,                loop2_1
    addi    $s4,        $s4,                1
    add     $s5,        $s4,                $s4
    ble     $s5,        $s1,                loop2

    jal     isAmicable
    nop     

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

# update code does not use this function
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
    addi    $sp,        $sp,                -4
    sw      $ra,        0($sp)

    li      $t0,        0                                   # $t0 i
loop_a:
    addi    $t0,        $t0,                1
    bgt     $t0,        $s1,                loop_a_end
    sll     $t1,        $t0,                2               # $t1 = i x 4
    add     $t1,        $s3,                $t1             # temp array[i] address $t1
    lw      $t2,        0($t1)                              # array[i] t2
    ble     $t2,        $t0,                loop_a
    ble     $t0,        $s0,                loop_a
    bgt     $t2,        $s1,                loop_a

    sll     $t3,        $t2,                2
    add     $t3,        $s3,                $t3             # temp array[array[i]] address $t3
    lw      $t4,        0($t3)
    bne     $t4,        $t0,                loop_a




# print i
    li      $v0,        1
    add     $a0,        $t0,                $zero
    syscall 

# print
    li      $v0,        4
    la      $a0,        output_and
    syscall 


# print array[i]
    li      $v0,        1
    add     $a0,        $t2,                $zero
    syscall 

# print
    li      $v0,        4
    la      $a0,        output_amicable
    syscall 

    addi    $s2,        $s2,                1

    ble     $t0,        $s1,                loop_a
loop_a_end:
    addi    $sp,        $sp,                4
    jr      $ra
