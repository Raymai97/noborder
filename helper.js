/* Slideshow */

var i_sshow = 0;
var i_sshow_max = 4;

var sshow_img_paths = [
	"pinball3D_small.jpg",
	"eternalSenia_small.jpg",
	"th075_small.jpg",
	"th08_small.jpg",
	"jewelChase_small.jpg"
];

var sshow_hd_img_paths = [
	"pinball3D.png",
	"eternalSenia.png",
	"th075.png",
	"th08.png",
	"jewelChase.png"	
];

var sshow_img_titles = [
	"Pinball 3D! (WinXP)",
	"Eternal Senia (nice game, find it on Steam for free)",
	"Touhou 7.5 Immaterial and Missing Power",
	"Touhou 8 Imperishable Night",
	"Jewel Chase (from Microsoft Entertainment Pack 1997)"
];

function OpenSShowHD() {
	window.open(sshow_hd_img_paths[i_sshow]);
}

function PrevSShow() {
	i_sshow -= 1;
	if (i_sshow < 0) {i_sshow = i_sshow_max;}
	UpdateSShow();
}

function NextSShow() {
	i_sshow += 1;
	if (i_sshow > i_sshow_max) {i_sshow = 0;}
	UpdateSShow();
}

function UpdateSShow() {
	var img_path = sshow_img_paths[i_sshow];
	var sshow = document.getElementById("sshow");
	sshow.style.backgroundImage = "url('" + img_path + "')";
	
	var img_title = sshow_img_titles[i_sshow];
	var sshowTitle = document.getElementById("sshowTitle");
	sshowTitle.textContent = img_title;
	sshowTitle.innerText = img_title;
}
