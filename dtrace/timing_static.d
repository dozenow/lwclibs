#pragma D option quiet



lwc:kern_lwc:create_tm:syslwc {
	@counts["create"] = count();
	@create[stringof(args[1][0])] = sum(args[0][0]);
	@create[stringof(args[1][1])] = sum(args[0][1]);
	@create[stringof(args[1][2])] = sum(args[0][2]);
	@create[stringof(args[1][3])] = sum(args[0][3]);
	@create[stringof(args[1][4])] = sum(args[0][4]);
	@create[stringof(args[1][5])] = sum(args[0][5]);
	@create[stringof(args[1][6])] = sum(args[0][6]);
	@create[stringof(args[1][7])] = sum(args[0][7]);
	@create[stringof(args[1][8])] = sum(args[0][8]);
}

lwc:kern_lwc:close_tm:syslwc {
	@counts["close"] = count();
	@close[stringof(args[1][0])] = sum(args[0][0]);
	@close[stringof(args[1][1])] = sum(args[0][1]);
	@close[stringof(args[1][2])] = sum(args[0][2]);
	@close[stringof(args[1][3])] = sum(args[0][3]);
	@close[stringof(args[1][4])] = sum(args[0][4]);
	@close[stringof(args[1][5])] = sum(args[0][5]);
	@close[stringof(args[1][6])] = sum(args[0][6]);
}

lwc:kern_lwc:snrelease_tm:syslwc {
	@counts["snrelease"] = count();
	@snrelease[stringof(args[1][0])] = sum(args[0][0]);
	@snrelease[stringof(args[1][1])] = sum(args[0][1]);
	@snrelease[stringof(args[1][2])] = sum(args[0][2]);
	@snrelease[stringof(args[1][3])] = sum(args[0][3]);
	@snrelease[stringof(args[1][4])] = sum(args[0][4]);
	@snrelease[stringof(args[1][5])] = sum(args[0][5]);
	@snrelease[stringof(args[1][6])] = sum(args[0][6]);
}

/* ::kern_lwc:freevm_tm:syslwc { */
/* 	@counts["freevm"] = count(); */
/* 	@freevm[stringof(args[1][0])] = sum(args[0][0]); */
/* 	@freevm[stringof(args[1][1])] = sum(args[0][1]); */
/* 	@freevm[stringof(args[1][2])] = sum(args[0][2]); */
/* 	@freevm[stringof(args[1][3])] = sum(args[0][3]); */
/* 	@freevm[stringof(args[1][4])] = sum(args[0][4]); */
/* 	@freevm[stringof(args[1][5])] = sum(args[0][5]); */
/* 	@freevm[stringof(args[1][6])] = sum(args[0][6]); */
/* 	@freevm[stringof(args[1][7])] = sum(args[0][7]); */
/* } */

/* ::kern_lwc:removepte_tm:syslwc { */
/* 	@counts["removepte"] = count(); */
/* 	@removepte[stringof(args[1][0])] = sum(args[0][0]); */
/* 	@removepte[stringof(args[1][1])] = sum(args[0][1]); */
/* 	@removepte[stringof(args[1][2])] = sum(args[0][2]); */
/* 	@removepte[stringof(args[1][3])] = sum(args[0][3]); */
/* 	@removepte[stringof(args[1][4])] = sum(args[0][4]); */
/* 	@removepte[stringof(args[1][5])] = sum(args[0][5]); */
/* 	@removepte[stringof(args[1][6])] = sum(args[0][6]); */
/* 	@removepte[stringof(args[1][7])] = sum(args[0][7]); */
/* } */



/* ::kern_lwc:freemap_tm:syslwc { */
/* 	@counts["freepmap"] = count(); */
/* 	@freepmap[stringof(args[1][0])] = sum(args[0][0]); */
/* 	@freepmap[stringof(args[1][1])] = sum(args[0][1]); */
/* 	@freepmap[stringof(args[1][2])] = sum(args[0][2]); */
/* 	@freepmap[stringof(args[1][3])] = sum(args[0][3]); */
/* 	@freepmap[stringof(args[1][4])] = sum(args[0][4]); */
/* 	@freepmap[stringof(args[1][5])] = sum(args[0][5]); */
/* 	@freepmap[stringof(args[1][6])] = sum(args[0][6]); */
/* 	@freepmap[stringof(args[1][7])] = sum(args[0][7]); */
/* } */


tick-1sec {
	printf("create timing:\n");
	printa("%40s %10@d\n", @create);
	printf("---------------------------------------\n");
	printf("close timing:\n");
	printa("%40s %10@d\n", @close);
	printf("snelease timing:\n");
	printa("%40s %10@d\n", @snrelease);
	
	/* printf("freevm timing:\n"); */
	/* printa("%40s %10@d\n", @freevm); */
	/* printf("freepmap timing:\n"); */
	/* printa("%40s %10@d\n", @freepmap); */
	/* printf("removepte timing:\n"); */
	/* printa("%40s %10@d\n", @removepte); */
	printf("Counts:\n");
 	printa("%40s %10@d\n", @counts);
	clear(@counts);
	/* clear(@freevm); */
	/* clear(@freepmap); */
	/* clear(@removepte); */
	clear(@create);
	clear(@close);
	clear(@snrelease);
}
