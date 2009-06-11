
type randstate = {mutable si:int;
		  mutable i:int}
let state = {si=0; i=0}

let advance_si () = 
  let si = state.si in
  let si1 = (si*22695477+1) land 2147483647 in
  state.si <- si1;
  state.i <- (state.i + 1)
      

let random () = 
  advance_si (); (* si=s_(i+1) *)
  (state.si lsr 16) land 16383

let set_seed seed = 
  state.si <- seed;
  advance_si (); (* i=1 *)
  advance_si (); (* i=2 *)
  advance_si (); (* i=3 *)
  ()


let test () = 
  set_seed 12345;
  for i = 0 to 99 do
    Printf.printf "%d\n" (random ())
  done

(* let _ = test () *)

