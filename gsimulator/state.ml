type pos = int*int

type dir = int (* 0..5 *)

type left_or_right = Left | Right

type sense_dir = 
    Here           (* sense the ant's current cell *)
  | Ahead          (* sense the cell straight ahead in the direction ant is facing *)
  | LeftAhead      (* sense the cell that would be ahead if ant turned left *)
  | RightAhead     (* sense the cell that would be ahead if ant turned right *)


type color = Red | Black

type marker = int (* uses bits 0..5 *)

type condition =
    Friend             (* cell contains an ant of the same color *)
  | Foe                (* cell contains an ant of the other color *)
  | FriendWithFood     (* cell contains an ant of the same color carrying food *)
  | FoeWithFood        (* cell contains an ant of the other color carrying food *)
  | Food               (* cell contains food (not being carried by an ant) *)
  | Rock               (* cell is rocky *)
  | Marker of (marker) (* cell is marked with a marker of this ant's color *)
  | FoeMarker          (* cell is marked with *some* marker of the other color *)
  | Home               (* cell belongs to this ant's anthill *)
  | FoeHome            (* cell belongs to the other anthill *)

type state = int

type instruction =
  | Sense of (sense_dir*state*state*condition)  
  | Mark of (marker*state)
  | Unmark of (marker*state)
  | PickUp of (state*state)
  | Drop of (state)
  | Turn of (left_or_right*state)
  | Move of (state*state)
  | Flip of(int*state*state)

type cell_kind = Rocky | Clear | Anthill of color

type ant = {color:color; 
	    id:int; 
	    mutable pos:pos; 
	    mutable carries_food:bool; 
	    mutable dead:bool; 
	    mutable resting:int; 
	    mutable state:int; 
	    mutable direction:int}

type cell = {kind:cell_kind; ant: ant option; food:int; marker:marker}

type game = {mutable ants:ant array; 
	     mutable world:cell array array;
	     mutable red_state_machine: instruction array;
	     mutable black_state_machine: instruction array;
	   }

let callback = ref None

let set_state_callback f = 
  callback := Some(f)

let game = {ants=[||]; world=[||]; red_state_machine=[||]; black_state_machine=[||]}

let rocky pos = 
  let x,y = pos in
  match game.world.(y).(x).kind with
  | Rocky -> true
  | _ -> false

let even x = x mod 2 == 0

let kind_at pos =
  let x,y = pos in
  game.world.(y).(x).kind

let some_ant_is_at pos = 
  let x,y = pos in
  match game.world.(y).(x).ant with 
  | None -> false
  | _ -> true

let color_at_pos_is pos c =
  let x,y = pos in
  match game.world.(y).(x).ant with 
  | Some(ant) -> ant.color = c
  | _ -> false

let ant_with_food_at pos =
  let x,y = pos in
  match game.world.(y).(x).ant with 
  | Some(ant) -> ant.carries_food
  | _ -> false

let food_at pos =
  let x,y = pos in
  game.world.(y).(x).food

let set_food_at pos food = 
  let x,y = pos in
  let cell = game.world.(y).(x) in
  game.world.(y).(x) <- {cell with food=food}

let anthill_at pos c = 
  let x,y = pos in
  game.world.(y).(x).kind = Anthill(c)

let get_ant_option_at pos = 
  let x,y = pos in
  game.world.(y).(x).ant

let set_marker_at pos c marker_num = 
  let x,y = pos in
  let shft = marker_num + 
    match c with
    | Red -> 6 
    | Black -> 0 
  in
  let mask = (1 lsl shft) in
  let cell = game.world.(y).(x) in
  game.world.(y).(x) <- {cell with marker = cell.marker lor mask}

let clear_marker_at pos c marker_num = 
  let x,y = pos in
  let shft = marker_num + 
    match c with
    | Red -> 6 
    | Black -> 0 
  in
  let mask = (1 lsl shft) in
  let cell = game.world.(y).(x) in
  game.world.(y).(x) <- {cell with marker = cell.marker land lnot mask}

let check_marker_at pos c marker_num = 
  let x,y = pos in
  let shft = marker_num + 
    match c with
    | Red -> 6 
    | Black -> 0 
  in
  let mask = (1 lsl shft) in
  (game.world.(y).(x).marker land mask) = mask
  
let check_any_marker_at pos c = 
  let f = (check_marker_at pos c) in
  (f 0) || (f 1) || (f 2) || (f 3) || (f 4) || (f 5)

let get_marker_list_at pos c = 
  let f = (check_marker_at pos c) in
  let rec loop i = 
    if i = 6 then
      []
    else if f i then 
      i::(loop (i+1))
    else
      loop (i+1)
  in
  loop 0

let string_of_cell_kind (x,y) =
  match game.world.(y).(x).kind with
    Rocky -> "Rocky"
  | Clear -> "Clear"
  | Anthill Red -> "Red anthill"
  | Anthill Black -> "Black anthill"

let string_of_marker_at pos c =
  List.fold_right (fun i str -> Printf.sprintf "%s%i" str i)
    (get_marker_list_at pos c) ""

let turn lr d = 
 match lr with
 | Left  ->  (d+5) mod 6
 | Right ->  (d+1) mod 6

let adjacent_cell p d = 
  let (x,y) = p in
  match d with
  | 0 -> (x+1, y)
  | 1 -> if even(y) then (x, y+1) else (x+1, y+1)
  | 2 -> if even(y) then (x-1, y+1) else (x, y+1)
  | 3 -> (x-1, y)
  | 4 -> if even(y) then (x-1, y-1) else (x, y-1)
  | 5 -> if even(y) then (x, y-1) else (x+1, y-1)
  | _ -> raise (Invalid_argument "bad direction")
      
let sensed_cell p d sd = 
  match sd with
  | Here -> p
  | Ahead -> adjacent_cell p d
  | LeftAhead -> adjacent_cell p (turn Left d)
  | RightAhead -> adjacent_cell p (turn Right d)
	
let other_color c = 
  match c with
  | Red -> Black
  | Black -> Red


let cell_matches p cond c = 
  if rocky(p) then 
    if cond = Rock then true else false
  else
    match cond with
    | Friend ->
	color_at_pos_is p c 
    | Foe ->
	color_at_pos_is p (other_color c)
    | FriendWithFood ->
	color_at_pos_is p c &&
	ant_with_food_at p 
    | FoeWithFood ->
	color_at_pos_is p (other_color c) &&
	ant_with_food_at p 
    | Food ->
        (food_at p) > 0
    | Rock ->
        false
    | Marker(i) ->
        (check_marker_at p c i)
    | FoeMarker ->
        check_any_marker_at p (other_color c)
    | Home ->
        anthill_at p c
    | FoeHome ->
        anthill_at p (other_color c)

let count_ants_at_with_color p = function
  | Some(c) -> 
      if color_at_pos_is p c then 
	1 
      else
	0
  | None -> 0
      
let count_adjacent_ants p c =
  (count_ants_at_with_color (adjacent_cell p 0) c) +
    (count_ants_at_with_color (adjacent_cell p 1) c) +
    (count_ants_at_with_color (adjacent_cell p 2) c) +
    (count_ants_at_with_color (adjacent_cell p 3) c) +
    (count_ants_at_with_color (adjacent_cell p 4) c) +
    (count_ants_at_with_color (adjacent_cell p 5) c)
    
let color_at pos = 
  let x,y = pos in
  match game.world.(y).(x).ant with
  | Some(ant) -> Some(ant.color)
  | _ -> None

let other_color_option = function 
  | None -> None
  | Some(c) -> Some(other_color c)

let move_ant_from_to old_pos new_pos = 
  let ox,oy = old_pos in
  let nx,ny = new_pos in
  let old_cell = game.world.(oy).(ox) in
  let new_cell = game.world.(ny).(nx) in
  if new_cell.ant = None then
    match old_cell.ant with 
    | None -> raise (Invalid_argument "no ant at this pos (cannot move)")
    | Some(ant) -> 
	begin
	  ant.pos <- new_pos;
	  game.world.(oy).(ox) <- {old_cell with ant=None};
	  game.world.(ny).(nx) <- {new_cell with ant=Some(ant)}
	end
  else
    raise (Invalid_argument "destination position not empty")

let kill_ant_at pos = 
  let x,y = pos in
  match game.world.(y).(x).ant with 
  | None -> raise (Invalid_argument "no ant at this pos (cannot kill)")
  | Some(ant) -> 
      ant.dead <- true;
      let cell = game.world.(y).(x) in
      game.world.(y).(x) <- { cell with 
			 ant = None; 
			 food = (cell.food + 3 + if ant_with_food_at pos then 1 else 0)
		       };
      true

(*
let set_food_at pos food = 
  let x,y = pos in
  let ant = game.world.(y).(x).ant in
  ant.dead <- true;
  game.world.(y).(x) <- {game.world.(y).(x) with food = food}
*)

let check_for_surrounded_ant_at p =
  let c = other_color_option (color_at p) in
  if (count_adjacent_ants p c) >= 5 then 
    kill_ant_at p
  else 
    false
	  
let check_for_surrounded_ants p =
  ignore(check_for_surrounded_ant_at p);
  for d = 0 to 5 do
    ignore(check_for_surrounded_ant_at (adjacent_cell p d))
  done

let ant_is_alive id = 
  not game.ants.(id).dead

let get_ant id =
  game.ants.(id)

let find_ant id =
  game.ants.(id).pos

let resting a = 
  a.resting

let set_resting a i = 
  a.resting <- i

let set_state a state = 
  a.state <- state

let get_instruction c state = 
  match c with
  | Red -> game.red_state_machine.(state)
  | Black -> game.black_state_machine.(state)


let randomint n = 
  (Icfprandom.random ()) mod n

let step id =
  if ant_is_alive id then
    let p = find_ant id in
    let a = get_ant id in
    if resting(a) > 0 then
      (set_resting a ((resting a) - 1))
    else
      begin 
	let instr = get_instruction a.color a.state in
	begin
	match instr with
	| Sense(sensedir, st1, st2, cond) ->
            let p' = sensed_cell p a.direction sensedir in
            let st = if cell_matches p' cond a.color then st1 else st2 in
            set_state a st
	| Mark(i, st) ->
            set_marker_at p a.color i;
            set_state a st
	| Unmark(i, st) ->
	    clear_marker_at p a.color i;
	    set_state a st
	| PickUp(st1, st2) ->
	    if a.carries_food || food_at(p) = 0 then
	      set_state a st2
	    else begin
	      set_food_at p (food_at(p) - 1);
	      a.carries_food <- true;
	      set_state a st1
	    end
	| Drop(st) ->
	    if a.carries_food then 
	      begin
		set_food_at p ((food_at p) + 1);
		a.carries_food <- false;
		()
	      end 
	    else
	      ();
	    set_state a st
	| Turn(lr, st) -> 
	    a.direction <- turn lr a.direction;
	    set_state a st
	| Move(st1, st2) ->
	    let x,y = a.pos in
	    let newp = adjacent_cell p a.direction in
	    if (rocky newp) || (some_ant_is_at newp) then
	      set_state a st2
	    else begin
	      move_ant_from_to p newp;
	      set_state a st1;
	      a.resting <- 14;
	      check_for_surrounded_ants newp
	    end 
	| Flip(p, st1, st2) ->
	    let st = if randomint(p) = 0 then st1 else st2 in
	    set_state a st
	end;
	match !callback with 
	| None -> ()
	| Some(cb) -> cb a instr
      end

let ant_list = ref []
let ant_id = ref 0

let generate_ant color p =
  let id = !ant_id in
  let ant = { color=color; id=id; pos=p; carries_food=false; dead=false;
	      resting=0; state=0; direction=0 } in
  ant_list := ant::!ant_list;
  ant_id := 1 + !ant_id;
  ant

let make_ants_array 
    ants = Array.of_list (List.rev !ant_list)
