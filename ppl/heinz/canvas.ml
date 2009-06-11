let x_dim = ref 25
let y_dim = ref 25
let x_size = 20.
let y_size = 25.

type color = Red | Black

let affine_rotate angle =
  let rad_angle = angle /. 180. *. acos (-1.) in
  let cos_a = cos rad_angle in
  let sin_a = sin rad_angle in
  [| cos_a ; sin_a ; ~-. sin_a ; cos_a ; 0. ; 0. |]

let affine_apply a x y =
  ( a.(0) *. x +. a.(2) *. y +. a.(4) ,
    a.(1) *. x +. a.(3) *. y +. a.(5) )

let affine_compose a1 a2 =
  [| a1.(0) *. a2.(0) +. a1.(1) *. a2.(2) ;
     a1.(0) *. a2.(1) +. a1.(1) *. a2.(3) ;
     a1.(2) *. a2.(0) +. a1.(3) *. a2.(2) ;
     a1.(2) *. a2.(1) +. a1.(3) *. a2.(3) ;
     a1.(4) *. a2.(0) +. a1.(5) *. a2.(2) +. a2.(4) ;
     a1.(4) *. a2.(1) +. a1.(5) *. a2.(3) +. a2.(5) ; |]

let affine_invert a =
  let r_det = 1. /. (a.(0) *. a.(3) -. a.(1) *. a.(2)) in
  [| a.(3) *. r_det ;
     ~-. (a.(1)) *. r_det ;
     ~-. (a.(2)) *. r_det ;
     a.(0) *. r_det ;
     (a.(2) *. a.(5) -. a.(3) *. a.(4)) *. r_det ;
     (a.(1) *. a.(4) -. a.(0) *. a.(5)) *. r_det ; |]

let affine_transl x y =
  [| 1. ; 0. ; 0. ; 1. ; x ; y |]

let affine_rotate_around_point x y angle =
  affine_compose
    (affine_compose
       (affine_transl (~-. x) (~-. y))
       (affine_rotate angle))
    (affine_transl x y)

let zoom_changed canvas adj () =
  canvas#set_pixels_per_unit adj#value

let setup_polygons root =
  let points = [| 210. ; 320.; 210.; 380.; 260.; 350.; |] in
  GnoCanvas.polygon ~points root
    ~props:( (`OUTLINE_COLOR "black") ::
	     if (new GnoCanvas.canvas root#canvas)#aa
	     then [ `FILL_COLOR_RGBA (Int32.of_int 0x0000ff80) ]
	     else [ `FILL_COLOR "blue" ; 
		    `FILL_STIPPLE (Gdk.Bitmap.create_from_data ~width:2 ~height:2 "\002\001") ] );
  let points = [|
    270.0; 330.0; 270.0; 430.0;
    390.0; 430.0; 390.0; 330.0;
    310.0; 330.0; 310.0; 390.0;
    350.0; 390.0; 350.0; 370.0;
    330.0; 370.0; 330.0; 350.0;
    370.0; 350.0; 370.0; 410.0;
    290.0; 410.0; 290.0; 330.0; |] in
  GnoCanvas.polygon ~points root
    ~props:[ `FILL_COLOR "tan" ; `OUTLINE_COLOR "black" ; `WIDTH_UNITS 3. ];
  ()

let create_ant root color =
  let points = [| x_size /. 5. *. 4.;       y_size /. 2.;
		  x_size /. 5.;             y_size /. 5. *. 4.;
		  x_size /. 5. *. 2.;       y_size /. 2.;
		  x_size /. 5.;             y_size /. 5. *. 1.;
		  x_size /. 5. *. 4.;       y_size /. 2.
		|]
  in
  GnoCanvas.polygon root
    ~props:[ `POINTS points; 
	     `FILL_COLOR (if color = Red then "red" else "black") ; 
	     `WIDTH_PIXELS 0; ]

let create_hexagon root x y =
  let points = [| x +. 0.;             y +. y_size /. 5.; 
		  x +. x_size /. 2.;  y +. 0.; 
		  x +. x_size;        y +. y_size /. 5.; 
		  x +. x_size;        y +. y_size /. 5. *. 4.;
		  x +. x_size /. 2.;  y +. y_size;
		  x +. 0.;             y +. y_size /. 5. *. 4.; 
		  x +. 0.;             y +. y_size /. 5.
		|]
  in
  GnoCanvas.polygon root
    ~props:[ `OUTLINE_COLOR "darkgreen"; `POINTS points; `FILL_COLOR "lightgreen"; `WIDTH_PIXELS 1; ]

let create_canvas window =
  let vbox = GPack.vbox ~border_width:4 ~spacing:4 ~packing:window#add () in
  let table = GPack.table ~rows:2 ~columns:2 ~packing:vbox#add ()
  in let canvas = GnoCanvas.canvas ~width:500 ~height:400 () in
  canvas#set_center_scroll_region false;
  setup_polygons canvas#root;
  for j = 0 to !x_dim - 1 do
    let adder = if (j mod 2) == 0 then 10 else 0
    in
    for i = 0 to !y_dim - 1 do
      create_hexagon canvas#root 
	(float_of_int (i*20 + adder)) (float_of_int (j*20));
    done
  done;
  let ant = create_ant canvas#root Red
  in
  ant#move 10. 0.;
  ant#affine_absolute [| 0.2; 1.; 1.; 0.2; 0.; 1. |];

  canvas#set_scroll_region 0. 0. 
    (x_size *. float_of_int (!x_dim) +. (x_size /. 5. *. 4.))
    (y_size *. float_of_int (!y_dim) /. 5. *. 4. +. (y_size /. 5. *. 2.));
  let frame = GBin.frame ~packing:window#add () in
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
  adj#connect#value_changed (zoom_changed canvas adj) ;
  GEdit.spin_button ~adjustment:adj ~rate:0. ~digits:2 ~width:50 
    ~packing:hbox#pack ()

let _ =
  let window = GWindow.window () in
  create_canvas window;
  window#connect#destroy ~callback:GMain.Main.quit ;
  window#show () ;
  GMain.Main.main ()
