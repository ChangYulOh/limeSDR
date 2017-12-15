ReadMe movingAver noise remover cpp..

This program is remove noise signal using moving average and etc.


for example..
    //if there is a set of points like below...
    // 3rd, 7th one is noise.
    //
    // 1 1 3 1 1 1 3 2 2 2
    //
    //     +       +
    //               + + +
    // + +   + + +
    //

    //third one might be noise..
    //this is how to configure..
    //
    //rule1. if meet flat values(similar;low variation), use as pivot points. (first pivot point's value is 1 at point 1,2)

    // get variations until some high value (more than twice) and skip the high value. (probably until 3th)
    //the variation of 3th is '2=(3-1)', and store previous average variation; ( point 1,2 average variation is 0, stored variation is 0)
    //then reset all 0, and do again.. 4'th, 5th, 6th...
    //4, 5, 6th variation also flat. then make pivot (current pivot point's 1 at point 4,5,6)

    //doing again....meet point7, (stored varation is 0, previous pivot is 1), meet point 8,9,10 (make current pivot as 2)
    //when current pivot(2) is higer than both previous pivot( '1' ) and stored variation( '0' ), there was some jump. at longist slope (point 7)

    //draw all pivots + jump slope  or only jump slope.
  
  
for compile in linux ubuntu 14.04

g++ -o point.out inflection_point.cpp -lm -std=c++11

excute
./point.out [sample filename]

//sample filename is consisted of x, y values. 
//like
x y
1 1
2 1
3 3
4 1
5 1
6 1
7 3
8 2
9 2
