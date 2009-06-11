exception Parse_error of (int option * string)

type instruction =
  | Sense of (sense_dir*state*state*condition)  
  | Mark of (marker*state)
  | Unmark of (marker*state)
  | PickUp of (state*state)
  | Drop of (state)
  | Turn of (left_or_right*state)
  | Move of (state*state)
  | Flip of(int*state*state)

let whitespace_regex = Str.regexp "[ \t]+"
let comment_regex = Str.regexp ";.*$"
let blank_regex = Str.regexp " "

let read_state_machine fname =
  let ifs max istr = 
    let i = int_of_string istr in
    if i > max then
      raise (Parse_error (None, (Printf.printf "out of range: %i > %i" i max)))
  in let ifs5 = ifs 5
  and ifsST ifs 99999
  and parse_sensedir = function
    | "Here" -> Here
    | "Ahead" -> Ahead
    | "LeftAhead" -> LeftAhead
    | "RightAhead" -> RightAhead
    | x -> raise Parse_error (None, (Printf.printf "illegal sensedir: %s" x))
  and parse_cond = function
    | "Friend" -> Friend
    | "Foe" -> Foe
    | "FriendWithFood" -> FriendWithFood
    | "FoeWithFood" -> FoeWithFood
    | "Food" -> Food
    | "Rock" -> Rock
    | "FoeMarker" -> FoeMarker
    | "Home" -> Home
    | "FoeHome" -> FoeHome
    | x -> raise Parse_error (None, (Printf.printf "illegal condition: %s" x))
  in let parse_line ich =
    match Str.split_delim whitespace_regex 
	(Str.replace_first comment_regex "" (input_string ich)) with
    | ["Sense"; sensedir; st1; st2; "Marker"; marker] ->
	Sense (parse_sensedir sensedir), (ifsST st1), (ifsST st2), 
	(Marker (ifs5 marker))
    | ["Sense"; sensedir; st1; st2; cond] ->
	Sense (parse_sensedir sensedir), (ifsST st1), (ifsST st2), 
	(parse_cond cond))
    | ["Mark"; i; st] ->
	Mark ((ifs5 i), (ifsST st))
    | ["Unmark"; i; st] ->
	Unmark ((ifs5 i), (ifsST st))
    | ["PickUp"; st1; st2] ->
	Unmark ((ifsST st1), (ifsST st2))
    | ["Drop"; st] ->
	Drop (ifsST st)
    | ["Turn"; "Left"; st] ->
	Turn (Left, (ifsST st))
    | ["Turn"; "Right"; st] ->
	Turn (Right, (ifsST st))
    | ["Move"; st1; st2] ->
	Move ((ifsST st1), (ifsST st2))
    | ["Flip"; p; st1; st2] ->
	Flip ((int_of_string p), (ifsST st1), (ifsST st2))
    | x :: _ ->
	raise (Parse_error (None, 
			    (Printf.printf "illegal instruction: %s" x)))
    | [] ->
	raise (Parse_error (None, "illegal empty line"))
  in let rec next_inst ich machine lnr =
    try
      next_inst ich (machine :: (parse_line ich)) (lnr + 1)
    with
      Parse_error (None, x) -> raise (Parse_error (Some lnr, x))
    | End_of_file -> Array.of_list (List.rev machine)
  in
  next_inst (open_in fname) [] 0

let generate_ant color p =
  { color=color; id=0; pos=p; carries_food=false; dead=true;
    resting=14;state=0;direction=0 }

let read_cartography fname =
  let ich = open_in fname
  in let x,y = (input_int ich), (input_int ich)
  in let world = Array.create_matrix x y {kind=Clear;ant=None;food=0;marker=0}
  and parse_char x y = function
    | '#' -> { kind=Rocky; ant=None; food=0; marker=0 }
    | '.' -> { kind=Clear; ant=None; food=0; marker=0 }
    | '0'..'9' as f -> 
	{ kind=Clear; ant=None; food=((int_from_char f) - 48); marker=0 }
    | '+' -> { kind=Anthill Red; ant=(generate_ant Red (x,y)); 
	       food=0; marker=0 }
    | '-' -> { kind=Anthill Black; ant=(generate_ant Black (x,y)); 
	       food=0; marker=0 }
    | x -> raise (Parse_error (None, Printf.printf "illegal char: %c" x))
  try
    for j = 0 to (y-1) do
      let line = Str.global_replace blank_regex "" (input_string ich)
      in
      for i = 0 to (x-1) do world.(i).(lnr) <- parse_char i j line.[i]; done
    done;
    begin
      try 
	ignore (input_string ich);
	raise (Parse_error (None, fname^": garbage in file:"))
      with
	End_of_file ->
	  close_out ich;
	  world
  with
    End_of_file -> raise (Parse_error (None, fname^": premature end of file))
