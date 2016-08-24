10 0 DO
  I print
LOOP

(n -- n')
: move
  dup 2 < IF
    1 +
  ELSE
    dup 4 > IF
      1 -
    ELSE drop
      3
    THEN
  THEN
;

10
BEGIN
  dup print
  1 -
dup 0 = UNTIL

10 BEGIN dup WHILE
  dup print
1 - REPEAT
