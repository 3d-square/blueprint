set id 2 ;

func add_two(v1, v2)
   set id id * 2 ;
   id dump
   set num 1 ;
   v1 + v2 + id ;
   return
end

set i2 add_two( 2 , 2 ) ;

i2 dump
