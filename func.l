set id 2 ;

func add_two(v1, v2)
   set id id * 2 ;
   id dump
   v1 + v2 + id ;
   return
end

del id

set tmp add_two( 2 , 2 ) ;
set i2 add_two( 2 , tmp ) ;


i2 dump
