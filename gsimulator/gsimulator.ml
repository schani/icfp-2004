type gant = {
    polygon: GnoCanvas.polygon;
    ant: State.ant;
  }

type ggame = {
    mutable game: State.game;
    mutable world_loaded: bool;
    mutable valid: bool;
    mutable ants: gant array;
    mutable init_antarray: State.ant array;
    mutable round: int;
  }

let ggame = { game = State.game; world_loaded = false; 
	      valid = false; ants =[||]; init_antarray = [||]; round = 0 }

let x_dim = ref 25
let y_dim = ref 25
let x_size = 20.
let y_size = 25.
let status_line = ref (GMisc.label ~text:"Statusline" ~justify:`FILL ())
let inspector_callback = ref (fun _ _ -> ())

let calc_pos (i, j) =
  let adder = if (j mod 2) == 1 then x_size /. 2. else 0.
  in
  ((float_of_int i) *. x_size +. adder), 
  ((float_of_int j) *. y_size /. 5. *. 4.)

let zoom_changed canvas adj () =
  canvas#set_pixels_per_unit adj#value

let create_ant root color (x, y) =
  let points = [| x +. x_size /. 5. *. 4.;  y +. y_size /. 2.;
		  x +. x_size /. 5.;        y +. y_size /. 5. *. 4.;
		  x +. x_size /. 5. *. 2.;  y +. y_size /. 2.;
		  x +. x_size /. 5.;        y +. y_size /. 5. *. 1.;
		  x +. x_size /. 5. *. 4.;  y +. y_size /. 2.
		|]
  in
  GnoCanvas.polygon root
    ~props:[ `POINTS points; 
	     `FILL_COLOR (if color = State.Red then "red" else "black") ; 
	     `WIDTH_PIXELS 0; ]

let create_hexagon kind root (x,y) =
  let points = [| x +. 0.;             y +. y_size /. 5.; 
		  x +. x_size /. 2.;  y +. 0.; 
		  x +. x_size;        y +. y_size /. 5.; 
		  x +. x_size;        y +. y_size /. 5. *. 4.;
		  x +. x_size /. 2.;  y +. y_size;
		  x +. 0.;             y +. y_size /. 5. *. 4.; 
		  x +. 0.;             y +. y_size /. 5.
		|]
  in
  let col1, col2 = match kind with
    State.Rocky -> "black", "darkgray"
  | State.Clear -> "darkgreen", "lightgreen"
  | State.Anthill State.Red -> "red", "orange"
  | State.Anthill State.Black -> "darkblue", "lightblue"
  in
  GnoCanvas.polygon root
    ~props:[ `OUTLINE_COLOR col1; `POINTS points; `FILL_COLOR col2; 
	     `WIDTH_PIXELS 1; ]

let modal_message what msg =
  let res = GWindow.message_dialog ~message:msg ~message_type:what 
      ~buttons:GWindow.Buttons.close ~modal:true () in
  ignore (res#run ());
  res#destroy ()

let create_grid world root ~x ~y =
  let grp = GnoCanvas.group ~x ~y root in
  for j = 0 to !x_dim - 1 do
    for i = 0 to !y_dim - 1 do
      ignore (create_hexagon (State.kind_at (i,j)) grp (calc_pos (i, j)));
    done
  done

let all_files () =
  let f = GFile.filter ~name:"All" () in
  f#add_pattern "*" ;
  f

let world_filter () = 
  let f = GFile.filter ~name:"World" () in
  f#add_pattern "*.world" ;
  f

let antworld_filter () = 
  let f = GFile.filter ~name:"Ant/World" () in
  f#add_pattern "*.world" ;
  f#add_pattern "*.ant" ;
  f

let ant_filter () = 
  let f = GFile.filter ~name:"Ant" () in
  f#add_pattern "*.ant" ;
  f

let load_state_machine color fname =
  try
    if color = State.Red then 
      ggame.game.State.red_state_machine <- Parser.read_state_machine fname
    else 
      ggame.game.State.black_state_machine <- Parser.read_state_machine fname;
  with
    Parser.Parse_error (None, msg) ->
      modal_message `ERROR (Printf.sprintf "somewhere in %s: %s\n" fname msg)
  | Parser.Parse_error (Some lnr, msg) ->
      modal_message `ERROR (Printf.sprintf "at %i in %s: %s\n" lnr fname msg)
  | Sys_error msg ->
      modal_message `ERROR (Printf.sprintf "error loading %s: %s\n" fname msg)
  | _ ->
      modal_message `ERROR (Printf.sprintf "unknown error loading %s" fname)

let antlist_deepcopy antlist =
  List.map (function a -> { a with State.id = a.State.id }) antlist
let antarray_deepcopy antarray =
  Array.map (function a -> { a with State.id = a.State.id }) antarray

let reset_ant root sant =
  { ant = { sant with State.id = sant.State.id };
    polygon = create_ant root sant.State.color (0., 0.);
  }

let update_gfx () =
  let update_ant a =
    let (x, y) = calc_pos (State.find_ant a.ant.State.id)
    in let matrix = 
      match a.ant.State.direction with
      | 0 -> [| 1.; 0.; 0.; 1.; x; y; |]
      | 1 -> [| 0.5; 0.866; 0.866; 0.5; x; y; |]
      | 2 -> [| -0.5; 0.866; -0.866; 0.5; x; y; |]
      | 3 -> [| -1.; 0.; 0.; -1.; x; y; |]
      | 4 -> [| -0.5; -0.866; 0.866; -0.5; x; y; |]
      | 5 -> [| 0.5; -0.866; 0.866; 0.5; x; y; |]
      | _ -> raise (Invalid_argument "Illegal direction in update_gfx")
    in
    a.polygon#affine_absolute matrix
  in
  Array.iter update_ant ggame.ants;
  !inspector_callback (Some (1,1)) None

let reset_game root () =
  if ggame.world_loaded then
    if Array.length ggame.game.State.red_state_machine > 0 then
      if Array.length ggame.game.State.black_state_machine > 0 then begin
	Array.iter (fun a -> a.polygon#destroy ()) ggame.ants;
	ggame.game.State.ants <- State.make_ants_array ();
	ggame.ants <- Array.map (reset_ant root) ggame.init_antarray;
	update_gfx ();
	ggame.valid <- true;
	ggame.round <- 0
      end else
	modal_message `ERROR "Can't reset: No black state machine loaded"
    else
      modal_message `ERROR "Can't reset: No red state machine loaded"
  else
    modal_message `ERROR "Can't reset: No world loaded"

let rec perform_step cnt () =
  if cnt = 0 then
    ()
  else begin
    ggame.round <- ggame.round + 1;
    if ggame.valid then begin
      for i = 0 to Array.length ggame.ants - 1 do
	State.step i
      done;
      update_gfx ()
    end else
      modal_message `INFO "Cant proceed: game is not valid";
    perform_step (cnt -1) ()
  end

let load_world canvas fname =
  try
    State.ant_list := [];
    State.ant_id := 0;
    ggame.game.State.world <- Parser.read_cartography fname;
    ggame.init_antarray <- antarray_deepcopy (Array.of_list !State.ant_list);
    ggame.world_loaded <- true;
    x_dim := Array.length ggame.game.State.world.(0);
    y_dim := Array.length ggame.game.State.world;
    create_grid ggame.game.State.world canvas#root ~x:0. ~y:0.;
  with
    Parser.Parse_error (None, msg) ->
      modal_message `ERROR (Printf.sprintf "somewhere in %s: %s\n" fname msg)
  | Parser.Parse_error (Some lnr, msg) ->
      modal_message `ERROR (Printf.sprintf "at %i in %s: %s\n" lnr fname msg)
  | Sys_error msg ->
      modal_message `ERROR (Printf.sprintf "error loading %s: %s\n" fname msg)
  | _ ->
      modal_message `ERROR (Printf.sprintf "unknown error loading %s" fname)

let rec load_file window canvas () =
  let dialog = GWindow.file_chooser_dialog 
      ~action:`OPEN 
      ~title:"Open File"
      ~parent:window () in
  let rec difi () =
	match dialog#filename with
	  Some x -> x
         | None -> difi () in
  dialog#add_button "Quit" `QUIT;
  dialog#add_button_stock `CANCEL `CANCEL ;
  dialog#add_button "Read World" `WORLD;
  if ggame.world_loaded then begin
    dialog#add_button "Red Code" `RED;
    dialog#add_button "Black Code" `BLACK;
    dialog#add_button "Red & Black Code" `BOTH;
  end;
  dialog#add_filter (antworld_filter ());
  dialog#add_filter (world_filter ()) ;
  dialog#add_filter (ant_filter ()) ;
  dialog#add_filter (all_files ()) ;
  begin match dialog#run () with
  | `DELETE_EVENT | `QUIT ->
      raise Exit
  | `CANCEL ->
      ()
  | `WORLD  ->
      load_world canvas (difi ())
  | `RED ->
      load_state_machine State.Red (difi ())
  | `BLACK ->
      load_state_machine State.Black (difi ())
  | `BOTH -> 
      load_state_machine State.Red (difi ());
      load_state_machine State.Black (difi ())
  end;
  dialog#destroy ();
  if ggame.world_loaded then
    window#show ()
  else
    load_file window canvas ()

let create_canvas window =
  let vbox = GPack.vbox ~border_width:4 ~spacing:4 ~packing:window#add () in
  let table = GPack.table ~rows:2 ~columns:2 ~packing:vbox#add ()
  in let canvas = GnoCanvas.canvas ~width:500 ~height:400 () in
  canvas#set_center_scroll_region false;
  canvas#set_scroll_region 0. 0. 
    (x_size *. float_of_int (!x_dim) +. (x_size /. 5. *. 4.))
    (y_size *. float_of_int (!y_dim) /. 5. *. 4. +. (y_size /. 5. *. 2.));
  let frame = GBin.frame () in
  table#attach ~left:0 ~right:1 ~top:0 ~bottom:1 ~expand:`BOTH ~fill:`BOTH 
    ~shrink:`BOTH ~xpadding:0 ~ypadding:0 frame#coerce;
  frame#add canvas#coerce;
  let w = GRange.scrollbar `HORIZONTAL ~adjustment:canvas#hadjustment () in
  table#attach ~left:0 ~right:1 ~top:1 ~bottom:2
    ~expand:`X ~fill:`BOTH ~shrink:`X ~xpadding:0 ~ypadding:0
    w#coerce ;
  let w = GRange.scrollbar `VERTICAL ~adjustment:canvas#vadjustment () in
  table#attach ~left:1 ~right:2 ~top:0 ~bottom:1
    ~expand:`Y ~fill:`BOTH ~shrink:`Y ~xpadding:0 ~ypadding:0 
    w#coerce;
  let hbox = GPack.hbox ~spacing:4 ~packing:vbox#pack () in
  let adj = GData.adjustment 
      ~value:1. ~lower:0.05 ~upper:5. 
      ~step_incr:0.05 ~page_incr:0.5 ~page_size:0.5 () in
  ignore (adj#connect#value_changed (zoom_changed canvas adj));
  ignore (GEdit.spin_button ~adjustment:adj ~rate:0. ~digits:2 ~width:50 
	    ~packing:hbox#pack ());
  let b = GButton.button ~stock:`OPEN ~label:"Load World" ~packing:hbox#pack ()
  in
  ignore (b#connect#clicked (load_file window canvas));
  let b = GButton.button ~label:"Reset" ~packing:hbox#pack ()
  in
  ignore (b#connect#clicked (reset_game canvas#root));
  let b = GButton.button ~label:"+1" ~packing:hbox#pack ()
  in
  ignore (b#connect#clicked (perform_step 1));
  let b = GButton.button ~label:"+5" ~packing:hbox#pack ()
  in
  ignore (b#connect#clicked (perform_step 5));
  let b = GButton.toggle_button ~label:"Run" ~packing:hbox#pack ()
  in let toggled () =
    let weiter = ref true
    in let stop_it = ref (fun () -> ())
    in
    Printf.fprintf stderr "TOGGLE\n";
    flush stderr;
    if b#active then begin
      Printf.fprintf stderr "  START\n";
      flush stderr;
      let the_id = GMain.Idle.add (fun () -> (perform_step 1) (); !weiter)
      in
      stop_it := fun () -> GMain.Idle.remove the_id
    end else begin
      Printf.fprintf stderr "  END\n";
      flush stderr;
      weiter := false;
      (!stop_it) ()
    end
  in
  ignore (b#connect#toggled toggled);
  let b = GButton.button ~label:"Update" ~packing:hbox#pack ()
  in
  ignore (b#connect#clicked update_gfx);
  hbox#pack !status_line#coerce;
  canvas

let create_inspector () =
  let window = GWindow.window () in
  let frame = GBin.frame ~label:"Tracking nothing" ~packing:window#add () in
  let vbox = GPack.vbox ~border_width:4 ~spacing:4 ~packing:frame#add () in
  let ci = GBin.frame ~label:"Cell" ~packing:vbox#pack () in
  let table = GPack.table ~rows:3 ~columns:2 ~packing:ci#add () in
  let ci_label_1 = GMisc.label ~text:"Cell kind:" () in
  table#attach ~left:0 ~right:1 ~top:0 ~bottom:1 
    ~xpadding:0 ~ypadding:0 ci_label_1#coerce;
  let ci_entry_1 = GEdit.entry ~text:"-" ~editable:false () in
  table#attach ~left:1 ~right:2 ~top:0 ~bottom:1 ~expand:`BOTH ~fill:`BOTH 
    ~xpadding:0 ~ypadding:0 ci_entry_1#coerce;
  let ci_label_2 = GMisc.label ~text:"Red Marker" () in
  table#attach ~left:0 ~right:1 ~top:1 ~bottom:2
    ~xpadding:0 ~ypadding:0 ci_label_2#coerce;
  let ci_entry_2 = GEdit.entry ~text:"-" ~editable:false () in
  table#attach ~left:1 ~right:2 ~top:1 ~bottom:2 ~expand:`BOTH ~fill:`BOTH 
    ~xpadding:0 ~ypadding:0 ci_entry_2#coerce;
  let ci_label_3 = GMisc.label ~text:"Black Marker" () in
  table#attach ~left:0 ~right:1 ~top:2 ~bottom:3
    ~xpadding:0 ~ypadding:0 ci_label_3#coerce;
  let ci_entry_3 = GEdit.entry ~text:"-" ~editable:false () in
  table#attach ~left:1 ~right:2 ~top:2 ~bottom:3 ~expand:`BOTH ~fill:`BOTH 
    ~xpadding:0 ~ypadding:0 ci_entry_3#coerce;
  let ai = GBin.frame ~label:"Ant" ~packing:vbox#pack () in
  let table = GPack.table ~rows:6 ~columns:2 ~packing:ai#add () in
  let ai_label_1 = GMisc.label ~text:"ID:" () in
  table#attach ~left:0 ~right:1 ~top:0 ~bottom:1 
    ~xpadding:0 ~ypadding:0 ai_label_1#coerce;
  let ai_entry_1 = GEdit.entry ~text:"-" ~editable:false () in
  table#attach ~left:1 ~right:2 ~top:0 ~bottom:1 ~expand:`BOTH ~fill:`BOTH 
    ~xpadding:0 ~ypadding:0 ai_entry_1#coerce;
  let ai_label_2 = GMisc.label ~text:"Color:" () in
  table#attach ~left:0 ~right:1 ~top:1 ~bottom:2 
    ~xpadding:0 ~ypadding:0 ai_label_2#coerce;
  let ai_entry_2 = GEdit.entry ~text:"-" ~editable:false () in
  table#attach ~left:1 ~right:2 ~top:1 ~bottom:2 ~expand:`BOTH ~fill:`BOTH 
    ~xpadding:0 ~ypadding:0 ai_entry_2#coerce;
  let ai_label_3 = GMisc.label ~text:"Food:" () in
  table#attach ~left:0 ~right:1 ~top:2 ~bottom:3 
    ~xpadding:0 ~ypadding:0 ai_label_3#coerce;
  let ai_entry_3 = GEdit.entry ~text:"-" ~editable:false () in
  table#attach ~left:1 ~right:2 ~top:2 ~bottom:3 ~expand:`BOTH ~fill:`BOTH 
    ~xpadding:0 ~ypadding:0 ai_entry_3#coerce;
  let ai_label_4 = GMisc.label ~text:"Resting:" () in
  table#attach ~left:0 ~right:1 ~top:3 ~bottom:4 
    ~xpadding:0 ~ypadding:0 ai_label_4#coerce;
  let ai_entry_4 = GEdit.entry ~text:"-" ~editable:false () in
  table#attach ~left:1 ~right:2 ~top:3 ~bottom:4 ~expand:`BOTH ~fill:`BOTH 
    ~xpadding:0 ~ypadding:0 ai_entry_4#coerce;
  let ai_label_5 = GMisc.label ~text:"State:" () in
  table#attach ~left:0 ~right:1 ~top:4 ~bottom:5 
    ~xpadding:0 ~ypadding:0 ai_label_5#coerce;
  let ai_entry_5 = GEdit.entry ~text:"-" ~editable:false () in
  table#attach ~left:1 ~right:2 ~top:4 ~bottom:5 ~expand:`BOTH ~fill:`BOTH 
    ~xpadding:0 ~ypadding:0 ai_entry_5#coerce;
  let ai_label_6 = GMisc.label ~text:"Direction:" () in
  table#attach ~left:0 ~right:1 ~top:5 ~bottom:6 
    ~xpadding:0 ~ypadding:0 ai_label_6#coerce;
  let ai_entry_6 = GEdit.entry ~text:"-" ~editable:false () in
  table#attach ~left:1 ~right:2 ~top:5 ~bottom:6 ~expand:`BOTH ~fill:`BOTH 
    ~xpadding:0 ~ypadding:0 ai_entry_6#coerce;
  let ins_callback pos ant =
    let pos = match (pos, ant) with
    | None, None
    | Some _, Some _ ->	raise (Invalid_argument "inspector_callback")
    | Some pos, None -> pos
    | None, Some id -> State.find_ant id
    in
    ci_entry_1#set_text (State.string_of_cell_kind pos);
    ci_entry_2#set_text (State.string_of_marker_at pos State.Red);
    ci_entry_3#set_text (State.string_of_marker_at pos State.Black);
(*    if (State.some_ant_is_at pos) then begin
      ai_entry_1#set_text (string_of_int (State.ant_at pos));
      ai_entry_2#set_text 
	(if color_at_pos_is pos State.Red then "Red" else "Black");
      ai_entry_3#set_text 
    end else begin
      ai_entry_1#set_text "-";
      ai_entry_2#set_text "-";
      ai_entry_3#set_text "-";
      ai_entry_4#set_text "-";
      ai_entry_5#set_text "-";
      ai_entry_6#set_text "-";
    end;*)
  in
  window#show ();
  inspector_callback := ins_callback

let _ =
  try
    let window = GWindow.window () in
    let canvas = create_canvas window in
    ignore (window#connect#destroy ~callback:GMain.Main.quit);
    create_inspector ();
    load_file window canvas ();
    GMain.Main.main ()
  with
    Exit -> ()
