set id 2 ;

func add_two(v1, v2)
   v1 + v2 + id ;
   return
end

set i2 add_two( 1 + 1 , 2 ) + add_two( 1 + 1 , 2 ) ;
i2 dump
id dump
