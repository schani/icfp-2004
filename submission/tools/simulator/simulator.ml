open State

let num_rounds = 100*1000

type dumpmode = DumpClassic | DumpWorldTraceFormat | NoDump
let dumpmode = ref NoDump
let show_sense_details = ref false

let dump rn = 
  let string_of_color = function
    | Red -> "red"
    | Black -> "black"
  in
  let rec string_of_marker_list str = function
    | [] -> str
    | m::ms -> string_of_marker_list (str^(string_of_int m)) ms
  in
  let num_food ant = (if ant.carries_food then 1 else 0) 
  in
  Printf.printf "\nAfter round %d...\n" rn;
  for y = 0 to Array.length game.world.(0) - 1 do
    for x = 0 to Array.length game.world - 1 do
      let pos = (x,y) in
      Printf.printf "cell (%d, %d): " x y;
      if rocky pos then
	Printf.printf "rock"
      else 
	begin
	  if food_at pos > 0 then Printf.printf "%d food; " (food_at pos);
	  if anthill_at pos Red then Printf.printf "red hill; ";
	  if anthill_at pos Black then Printf.printf "black hill; ";
	  if check_any_marker_at pos Red then 
	    Printf.printf "red marks: %s; " (string_of_marker_list "" (get_marker_list_at pos Red));
	  if check_any_marker_at pos Black then 
	    Printf.printf "black marks: %s; " (string_of_marker_list "" (get_marker_list_at pos Black));
	  match get_ant_option_at pos with
	  | None -> ()
	  | Some(ant) -> 
	      Printf.printf "%s ant of id %d, dir %d, food %d, state %d, resting %d" 
		(string_of_color ant.color) ant.id ant.direction (num_food ant) ant.state ant.resting;
	end;
      Printf.printf "\n"
    done
  done

let world_trace rn = 
  Printf.printf "#\n"

let world_tracer_callback ant instr success = 
  let string_of_ant ant = 
    match ant.color with
    | Red   -> "0 "^(string_of_int ant.id)
    | Black -> "1 "^(string_of_int ant.id)
  in
  let string_of_dir = function
    | Here -> "h"
    | Ahead -> "a"
    | LeftAhead -> "l"
    | RightAhead -> "r"
  in
  let string_of_cond = function
    | _ when (not !show_sense_details) -> ""
    | Friend -> " friend"
    | Foe -> "  foe"
    | FriendWithFood -> " friendfood"
    | FoeWithFood -> " foefood"
    | Food -> " food"
    | Rock -> " rock"
    | Marker(m) -> (" mark"^(string_of_int m))
    | FoeMarker -> " foemark"
    | Home -> " home"
    | FoeHome -> " foehome"
  in
  let string_of_instr = function
    | Move(_,_)     -> "mf"
    | Turn(Left,_)  -> "tl"
    | Turn(Right,_) -> "tr"
    | PickUp(_,_)   -> "pf"
    | Drop(_)       -> "df"
    | Mark(m,_)   -> "s"^(string_of_int m)
    | Unmark(m,_)   -> "c"^(string_of_int m)
    | Sense(dir, _, _, cond) -> "l"^(string_of_dir dir)^(string_of_cond cond)
    | _ -> ""
  in
  let string_of_cell cell = 
    match cell.kind with
    | Anthill(Red) -> "red hill"
    | Anthill(Black) -> "black hill"
    | _ -> ""
  in
  let string_of_state ant =
    (string_of_int ant.state)
  in
  let succstr = (if success then "" else "!") in
  let x,y = ant.pos in
  match instr with
  | Flip(_) -> ()
  | _ ->
      Printf.printf "%s%s %s %s\n" (succstr) (string_of_ant ant) (string_of_instr instr) (string_of_state ant)
(*      Printf.printf "%s %s %s %s\n" (string_of_ant ant) (string_of_instr instr) (string_of_state ant) (string_of_cell game.world.(y).(x)) *)
	
let world_tracer_ant_death_callback ant =
  let string_of_ant ant = 
    match ant.color with
    | Red   -> "0 "^(string_of_int ant.id)
    | Black -> "1 "^(string_of_int ant.id)
  in
  Printf.printf  "%s dt\n" (string_of_ant ant) 
    

let determine_winner () = 
  let add i j = 
    i := !i+j
  in
  let black_food = ref 0 in
  let red_food = ref 0 in
  for x = 0 to Array.length game.world - 1 do
    for y = 0 to Array.length game.world.(0) - 1 do
      if anthill_at (x,y) Red then add red_food (food_at (x,y));
      if anthill_at (x,y) Black then add black_food (food_at (x,y));
    done
  done;
  Printf.fprintf stderr "Red collected:   %d\nBlack collected: %d\n" !red_food !black_food;
  if !red_food == !black_food then
    Printf.fprintf stderr "Draw\n"
  else if !red_food > !black_food then
    Printf.fprintf stderr "Red\n"
  else
    Printf.fprintf stderr "Black\n"

let main_loop () = 
  try
  begin
    match !dumpmode with
    | DumpClassic -> ()
    | DumpWorldTraceFormat -> 
	begin
	  set_state_callback world_tracer_callback;
	  set_state_death_callback world_tracer_ant_death_callback;
	end
    | NoDump -> ()
  end;
    for round_number = 0 to num_rounds do 
      begin
	match !dumpmode with
	| DumpClassic -> dump round_number;
	| DumpWorldTraceFormat -> world_trace round_number;
	| NoDump -> ();
      end;
      for i = 0 to Array.length game.ants - 1 do
	try
	  step i
	with
	  x -> Printf.fprintf stderr 
	      "unhandled exeption in step %i at round %i: %s\n"
	      i round_number (Printexc.to_string x);
	    raise x
      done;
    done;
    determine_winner ()
  with
    x -> Printf.fprintf stderr "unhandled exeption in main_loop: %s\n"
	(Printexc.to_string x);
      raise x

let usage () = 
  failwith ("USAGE: "^Sys.argv.(0)^" <map> <red> <black> <dumpmode>\ndumpmode=no,wtf,classic\n")

let set_dump_mode str = 
  match str with
  | "no" -> dumpmode := NoDump
  | "wtf" -> dumpmode := DumpWorldTraceFormat
  | "classic" -> dumpmode := DumpClassic
  | _ -> usage()

let _ = 
  try
    Printf.fprintf stderr "GDG\n";
    if Array.length Sys.argv < 4 || Array.length Sys.argv > 5 then usage ();
    if (Array.length Sys.argv =5) then set_dump_mode Sys.argv.(4);
    let seed = 12345 in
    Icfprandom.set_seed seed;
    if !dumpmode = DumpClassic then Printf.printf "random seed: %d\n" seed;
    game.world <- Parser.read_cartography Sys.argv.(1);
    game.red_state_machine <- Parser.read_state_machine Sys.argv.(2);
    game.black_state_machine <- Parser.read_state_machine Sys.argv.(3);
    game.ants <- make_ants_array ();
    main_loop ()
  with
  | Parser.Parse_error(Some(line), msg) -> Printf.fprintf stderr "%s in line %d\n" msg line
  | Parser.Parse_error(None, msg) -> Printf.fprintf stderr "%s in unknown line" msg
	
  
