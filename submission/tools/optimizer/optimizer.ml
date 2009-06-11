(* $Id: optimizer.ml,v 1.1 2004/06/07 12:51:08 schani Exp $
 * Liesegang Optimizer
 *)

open State
open Printf

type codeline = {
    id: int;
    mutable new_id: int;
    mutable instruction: instruction;
    comment: string;
    mutable live: bool;
  }

let argv0 = Sys.argv.(0)
let do_debug = ref false

let instruction_is_goto = function
  | Sense (_, a, b, _) when a = b -> true
  | Flip (_, a, b) when a = b -> true
  | _ -> false

let instruction_targets = function
  | Sense (_, a, b, _)
  | PickUp (a, b)
  | Move (a, b)
  | Flip (_, a, b) -> a, b
  | Mark (_, a)
  | Unmark (_, a)
  | Drop (a)
  | Turn (_, a) -> a, a

let codeline_redirect cl (a, b) =
  match cl.instruction with
  | Sense (x, _, _, y) -> cl.instruction <- Sense (x, a, b, y)
  | PickUp (_, _) -> cl.instruction <- PickUp (a, b)
  | Move (_, _) -> cl.instruction <- Move (a, b)
  | Flip (x, _, _) -> cl.instruction <- Flip (x, a, b)
  | Mark (x, _) -> cl.instruction <- Mark (x, a)
  | Unmark (x, _) -> cl.instruction <- Unmark (x, a)
  | Drop (_) -> cl.instruction <- Drop (a)
  | Turn (x, _) -> cl.instruction <- Turn (x, a)

let codeline_generate nr codeline =
  { id = nr;
    new_id = -1;
    instruction = codeline.Parser.instruction;
    comment = codeline.Parser.comment;
    live = false;
  }

let dump_line cl =
  Printf.fprintf stderr "[%c]%5i(%5i): %s%s\n" (if cl.live then '+' else ' ')
    cl.id cl.new_id (string_of_instruction cl.instruction) cl.comment;
  flush stderr

let rec mark_lifeness cla index =
  let cl = cla.(index) 
  in
  if not cl.live then begin
    cl.live <- true;
    let a,b = instruction_targets cl.instruction
    in
    if a <> b then mark_lifeness cla b;
    mark_lifeness cla a
  end

let rec reorder ?(i = 0) = function
    [] -> ()
  | cl :: rest ->
      if cl.live then begin
	cl.new_id <- i;
	reorder ~i:(i+1) rest
      end else
	reorder ~i rest

let recode cla cl =
  if cl.live then
    let a,b = instruction_targets cl.instruction
    in
    codeline_redirect cl (cla.(a).new_id, cla.(b).new_id)

let reassign_target src dest cl =
  if cl.live then
    let a,b = instruction_targets cl.instruction
    in 
    if a = src || b = src then begin
      let a2 = if a = src then dest else a
      and b2 = if b = src then dest else b
      in
      codeline_redirect cl (a2, b2);
    end

let common_sub_expr cla cl =
  let check_expr cl2 =
    if cl2.live then
      if cl <> cl2 then begin
	if cl.instruction = cl2.instruction then begin
	  cl2.live <- false;
	  cl2.new_id <- -1;
	  Array.iter (reassign_target cl2.id cl.id) cla;
	end
      end
  in
  if cl.live then
    Array.iter check_expr cla

let rec redirect cla cl =
  if cl.live then begin
    let a1,b1 = instruction_targets cl.instruction
    in let a2 =
      if instruction_is_goto cla.(a1).instruction then
	fst (instruction_targets cla.(a1).instruction)
      else
	if cla.(a1).live then
	  a1
	else
	  raise (Failure "redirect: should not happen")
    and b2 = 
      if instruction_is_goto cla.(b1).instruction then
	snd (instruction_targets cla.(b1).instruction)
      else
	if cla.(b1).live then
	  b1
	else
	  raise (Failure "redirect: should not happen")
    in
    codeline_redirect cl (a2, b2);
    if a1 <> a2 || b1 <> b2 then
      redirect cla cl
  end

let filter cla =
  let ht = Hashtbl.create 1000
  in let rec genlist i l =
    try
      genlist (i+1) ((Hashtbl.find ht i) :: l)
    with
      Not_found -> l
  in
  Array.iter (fun cl -> if cl.new_id <> -1 then 
    Hashtbl.add ht cl.new_id cl) cla;
  Array.of_list (List.rev (genlist 0 []))

let debug_dump cla =
  Array.iter dump_line cla

let print_result cla =
  for i = 0 to Array.length cla - 1 do
    let cl = cla.(i)
    in
    print_string ((string_of_instruction cl.instruction)^(cl.comment)^"\n");
  done

let revamp_cl cl =
  { id = cl.new_id;
    new_id = -1;
    instruction = cl.instruction;
    comment = cl.comment;
    live = false;
  }

let perform_pass cla nr =
  fprintf stderr "pass %i begin (%i states)\n" nr (Array.length cla); 
  flush stderr;
  fprintf stderr "analyzing lifeness\n"; flush stderr;
  mark_lifeness cla 0;
  cla.(0).live <- true;
  fprintf stderr "common sub states\n"; flush stderr;
  Array.iter (common_sub_expr cla) cla;
  fprintf stderr "reordering\n"; flush stderr;
  reorder (Array.to_list cla);
  fprintf stderr "redirecting\n"; flush stderr;
  Array.iter (redirect cla) cla;
  fprintf stderr "recoding\n"; flush stderr;
  Array.iter (recode cla) cla;
  if !do_debug then
    debug_dump cla;
  Array.map revamp_cl (filter cla)

let optimize cla =
  let rec _optimize pass_nr old_cnt cla_old =
    let cla_new = perform_pass cla_old pass_nr
    in
    if old_cnt > Array.length cla_new then
      _optimize (pass_nr + 1) (Array.length cla_new) cla_new
    else
      cla_new
  in let opt_cla = _optimize 1 (Array.length cla) cla
  in
  fprintf stderr "optimization finished (%i states)\n" (Array.length opt_cla); 
  flush stderr;
  print_result opt_cla
  

let _ =
  if Array.length Sys.argv = 1 then begin
    fprintf stderr "%s: usage: %s <source.lsg>\n" argv0 argv0;
    exit 1
  end else begin
    if Array.length Sys.argv = 3 then
      do_debug := true;
    let c = Array.mapi codeline_generate (Parser.read_code_lines Sys.argv.(1))
    in
    optimize c
  end
