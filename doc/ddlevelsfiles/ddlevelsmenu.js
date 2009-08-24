//** All Levels Navigational Menu- (c) Dynamic Drive DHTML code library: http://www.dynamicdrive.com
//** Script Download/ instructions page: http://www.dynamicdrive.com/dynamicindex1/ddlevelsmenu/
//** Usage Terms: http://www.dynamicdrive.com/notice.htm

//** Current version: 2.2 See changelog.txt for details

if (typeof dd_domreadycheck=="undefined") //global variable to detect if DOM is ready
	var dd_domreadycheck=false

var ddlevelsmenu={

enableshim: true, //enable IFRAME shim to prevent drop down menus from being hidden below SELECT or FLASH elements? (tip: disable if not in use, for efficiency)

arrowpointers:{
	downarrow: ["ddlevelsfiles/arrow-down.gif", 11,7], //[path_to_down_arrow, arrowwidth, arrowheight]
	rightarrow: ["ddlevelsfiles/arrow-right.gif", 12,12], //[path_to_right_arrow, arrowwidth, arrowheight]
	showarrow: {toplevel: true, sublevel: true} //Show arrow images on top level items and sub level items, respectively?
},
hideinterval: 0, //delay in milliseconds before entire menu disappears onmouseout.
effects: {enableswipe: false, enablefade: false, duration: 200},
httpsiframesrc: "blank.htm", //If menu is run on a secure (https) page, the IFRAME shim feature used by the script should point to an *blank* page *within* the secure area to prevent an IE security prompt. Specify full URL to that page on your server (leave as is if not applicable).

///No need to edit beyond here////////////////////

topmenuids: [], //array containing ids of all the primary menus on the page
topitems: {}, //object array containing all top menu item links
subuls: {}, //object array containing all ULs
lastactivesubul: {}, //object object containing info for last mouse out menu item's UL
topitemsindex: -1,
ulindex: -1,
hidetimers: {}, //object array timer
shimadded: false,
nonFF: !/Firefox[\/\s](\d+\.\d+)/.test(navigator.userAgent), //detect non FF browsers
getoffset:function(what, offsettype){
	return (what.offsetParent)? what[offsettype]+this.getoffset(what.offsetParent, offsettype) : what[offsettype]
},

getoffsetof:function(el){
	el._offsets={left:this.getoffset(el, "offsetLeft"), top:this.getoffset(el, "offsetTop")}
},

getwindowsize:function(){
	this.docwidth=window.innerWidth? window.innerWidth-10 : this.standardbody.clientWidth-10
	this.docheight=window.innerHeight? window.innerHeight-15 : this.standardbody.clientHeight-18
},

gettopitemsdimensions:function(){
	for (var m=0; m<this.topmenuids.length; m++){
		var topmenuid=this.topmenuids[m]
		for (var i=0; i<this.topitems[topmenuid].length; i++){
			var header=this.topitems[topmenuid][i]
			var submenu=document.getElementById(header.getAttribute('rel'))
			header._dimensions={w:header.offsetWidth, h:header.offsetHeight, submenuw:submenu.offsetWidth, submenuh:submenu.offsetHeight}
		}
	}
},

isContained:function(m, e){
	var e=window.event || e
	var c=e.relatedTarget || ((e.type=="mouseover")? e.fromElement : e.toElement)
	while (c && c!=m)try {c=c.parentNode} catch(e){c=m}
	if (c==m)
		return true
	else
		return false
},

addpointer:function(target, imgclass, imginfo, BeforeorAfter){
	var pointer=document.createElement("img")
	pointer.src=imginfo[0]
	pointer.style.width=imginfo[1]+"px"
	pointer.style.height=imginfo[2]+"px"
	if(imgclass=="rightarrowpointer"){
		pointer.style.left=target.offsetWidth-imginfo[2]-2+"px"
	}
	pointer.className=imgclass
	var target_firstEl=target.childNodes[target.firstChild.nodeType!=1? 1 : 0] //see if the first child element within A is a SPAN (found in sliding doors technique)
	if (target_firstEl && target_firstEl.tagName=="SPAN"){
		target=target_firstEl //arrow should be added inside this SPAN instead if found
	}
	if (BeforeorAfter=="before")
		target.insertBefore(pointer, target.firstChild)
	else
		target.appendChild(pointer)
},

css:function(el, targetclass, action){
	var needle=new RegExp("(^|\\s+)"+targetclass+"($|\\s+)", "ig")
	if (action=="check")
		return needle.test(el.className)
	else if (action=="remove")
		el.className=el.className.replace(needle, "")
	else if (action=="add" && !needle.test(el.className))
		el.className+=" "+targetclass
},

addshimmy:function(target){
	var shim=(!window.opera)? document.createElement("iframe") : document.createElement("div") //Opera 9.24 doesnt seem to support transparent IFRAMEs
	shim.className="ddiframeshim"
	shim.setAttribute("src", location.protocol=="https:"? this.httpsiframesrc : "about:blank")
	shim.setAttribute("frameborder", "0")
	target.appendChild(shim)
	try{
		shim.style.filter='progid:DXImageTransform.Microsoft.Alpha(style=0,opacity=0)'
	}
	catch(e){}
	return shim
},

positionshim:function(header, submenu, dir, scrollX, scrollY){
	if (header._istoplevel){
		var scrollY=window.pageYOffset? window.pageYOffset : this.standardbody.scrollTop
		var topgap=header._offsets.top-scrollY
		var bottomgap=scrollY+this.docheight-header._offsets.top-header._dimensions.h
		if (topgap>0){
			this.shimmy.topshim.style.left=scrollX+"px"
			this.shimmy.topshim.style.top=scrollY+"px"
			this.shimmy.topshim.style.width="99%"
			this.shimmy.topshim.style.height=topgap+"px" //distance from top window edge to top of menu item
		}
		if (bottomgap>0){
			this.shimmy.bottomshim.style.left=scrollX+"px"
			this.shimmy.bottomshim.style.top=header._offsets.top + header._dimensions.h +"px"
			this.shimmy.bottomshim.style.width="99%"
			this.shimmy.bottomshim.style.height=bottomgap+"px" //distance from bottom of menu item to bottom window edge
		}
	}
},

hideshim:function(){
	this.shimmy.topshim.style.width=this.shimmy.bottomshim.style.width=0
	this.shimmy.topshim.style.height=this.shimmy.bottomshim.style.height=0
},


buildmenu:function(mainmenuid, header, submenu, submenupos, istoplevel, dir){
	header._master=mainmenuid //Indicate which top menu this header is associated with
	header._pos=submenupos //Indicate pos of sub menu this header is associated with
	header._istoplevel=istoplevel
	if (istoplevel){
		this.addEvent(header, function(e){
		ddlevelsmenu.hidemenu(ddlevelsmenu.subuls[this._master][parseInt(this._pos)])
		}, "click")
	}
	this.subuls[mainmenuid][submenupos]=submenu
	header._dimensions={w:header.offsetWidth, h:header.offsetHeight, submenuw:submenu.offsetWidth, submenuh:submenu.offsetHeight}
	this.getoffsetof(header)
	submenu.style.left=0
	submenu.style.top=0
	submenu.style.visibility="hidden"
	this.addEvent(header, function(e){ //mouseover event
		if (!ddlevelsmenu.isContained(this, e)){
			var submenu=ddlevelsmenu.subuls[this._master][parseInt(this._pos)]
			if (this._istoplevel){
				ddlevelsmenu.css(this, "selected", "add")
			clearTimeout(ddlevelsmenu.hidetimers[this._master][this._pos])
			}
			ddlevelsmenu.getoffsetof(header)
			var scrollX=window.pageXOffset? window.pageXOffset : ddlevelsmenu.standardbody.scrollLeft
			var scrollY=window.pageYOffset? window.pageYOffset : ddlevelsmenu.standardbody.scrollTop
			var submenurightedge=this._offsets.left + this._dimensions.submenuw + (this._istoplevel && dir=="topbar"? 0 : this._dimensions.w)
			var submenubottomedge=this._offsets.top + this._dimensions.submenuh
			//Sub menu starting left position
			var menuleft=(this._istoplevel? this._offsets.left + (dir=="sidebar"? this._dimensions.w : 0) : this._dimensions.w)
			if (submenurightedge-scrollX>ddlevelsmenu.docwidth){
				menuleft+= -this._dimensions.submenuw + (this._istoplevel && dir=="topbar" ? this._dimensions.w : -this._dimensions.w)
			}
			submenu.style.left=menuleft+"px"
			//Sub menu starting top position
			var menutop=(this._istoplevel? this._offsets.top + (dir=="sidebar"? 0 : this._dimensions.h) : this.offsetTop)
			if (submenubottomedge-scrollY>ddlevelsmenu.docheight){ //no room downwards?
				if (this._dimensions.submenuh<this._offsets.top+(dir=="sidebar"? this._dimensions.h : 0)-scrollY){ //move up?
					menutop+= - this._dimensions.submenuh + (this._istoplevel && dir=="topbar"? -this._dimensions.h : this._dimensions.h)
				}
				else{ //top of window edge
					menutop+= -(this._offsets.top-scrollY) + (this._istoplevel && dir=="topbar"? -this._dimensions.h : 0)
				}
			}
			submenu.style.top=menutop+"px"
			if (ddlevelsmenu.enableshim && (ddlevelsmenu.effects.enableswipe==false || ddlevelsmenu.nonFF)){ //apply shim immediately only if animation is turned off, or if on, in non FF2.x browsers
				ddlevelsmenu.positionshim(header, submenu, dir, scrollX, scrollY)
			}
			else{
				submenu.FFscrollInfo={x:scrollX, y:scrollY}
			}
			ddlevelsmenu.showmenu(header, submenu, dir)
		}
	}, "mouseover")
	this.addEvent(header, function(e){ //mouseout event
		var submenu=ddlevelsmenu.subuls[this._master][parseInt(this._pos)]
		if (this._istoplevel){
			if (!ddlevelsmenu.isContained(this, e) && !ddlevelsmenu.isContained(submenu, e)) //hide drop down ul if mouse moves out of menu bar item but not into drop down ul itself
				ddlevelsmenu.hidemenu(submenu)
		}
		else if (!this._istoplevel && !ddlevelsmenu.isContained(this, e)){
			ddlevelsmenu.hidemenu(submenu)
		}

	}, "mouseout")
},

setopacity:function(el, value){
	el.style.opacity=value
	if (typeof el.style.opacity!="string"){ //if it's not a string (ie: number instead), it means property not supported
		el.style.MozOpacity=value
		if (el.filters){
			el.style.filter="progid:DXImageTransform.Microsoft.alpha(opacity="+ value*100 +")"
		}
	}
},

showmenu:function(header, submenu, dir){
	if (this.effects.enableswipe || this.effects.enablefade){
		if (this.effects.enableswipe){
			var endpoint=(header._istoplevel && dir=="topbar")? header._dimensions.submenuh : header._dimensions.submenuw
			submenu.style.width=submenu.style.height=0
			submenu.style.overflow="hidden"
		}
		if (this.effects.enablefade){
			this.setopacity(submenu, 0) //set opacity to 0 so menu appears hidden initially
		}
		submenu._curanimatedegree=0
		submenu.style.visibility="visible"
		clearInterval(submenu._animatetimer)
		submenu._starttime=new Date().getTime() //get time just before animation is run
		submenu._animatetimer=setInterval(function(){ddlevelsmenu.revealmenu(header, submenu, endpoint, dir)}, 10)
	}
	else{
		submenu.style.visibility="visible"
	}
},

revealmenu:function(header, submenu, endpoint, dir){
	var elapsed=new Date().getTime()-submenu._starttime //get time animation has run
	if (elapsed<this.effects.duration){
		if (this.effects.enableswipe){
			if (submenu._curanimatedegree==0){ //reset either width or height of sub menu to "auto" when animation begins
				submenu.style[header._istoplevel && dir=="topbar"? "width" : "height"]="auto"
			}
			submenu.style[header._istoplevel && dir=="topbar"? "height" : "width"]=(submenu._curanimatedegree*endpoint)+"px"
		}
		if (this.effects.enablefade){
			this.setopacity(submenu, submenu._curanimatedegree)
		}
	}
	else{
		clearInterval(submenu._animatetimer)
		if (this.effects.enableswipe){
			submenu.style.width="auto"
			submenu.style.height="auto"
			submenu.style.overflow="visible"
		}
		if (this.effects.enablefade){
			this.setopacity(submenu, 1)
			submenu.style.filter=""
		}
		if (this.enableshim && submenu.FFscrollInfo) //if this is FF browser (meaning shim hasn't been applied yet
			this.positionshim(header, submenu, dir, submenu.FFscrollInfo.x, submenu.FFscrollInfo.y)
	}
	submenu._curanimatedegree=(1-Math.cos((elapsed/this.effects.duration)*Math.PI)) / 2
},

hidemenu:function(submenu){
	if (typeof submenu._pos!="undefined"){ //if submenu is outermost UL drop down menu
		this.css(this.topitems[submenu._master][parseInt(submenu._pos)], "selected", "remove")
		if (this.enableshim)
			this.hideshim()
	}
	clearInterval(submenu._animatetimer)
	submenu.style.left=0
	submenu.style.top="-1000px"
	submenu.style.visibility="hidden"
},


addEvent:function(target, functionref, tasktype) {
	if (target.addEventListener)
		target.addEventListener(tasktype, functionref, false);
	else if (target.attachEvent)
		target.attachEvent('on'+tasktype, function(){return functionref.call(target, window.event)});
},

domready:function(functionref){ //based on code from the jQuery library
	if (dd_domreadycheck){
		functionref()
		return
	}
	// Mozilla, Opera and webkit nightlies currently support this event
	if (document.addEventListener) {
		// Use the handy event callback
		document.addEventListener("DOMContentLoaded", function(){
			document.removeEventListener("DOMContentLoaded", arguments.callee, false )
			functionref();
			dd_domreadycheck=true
		}, false )
	}
	else if (document.attachEvent){
		// If IE and not an iframe
		// continually check to see if the document is ready
		if ( document.documentElement.doScroll && window == window.top) (function(){
			if (dd_domreadycheck){
				functionref()
				return
			}
			try{
				// If IE is used, use the trick by Diego Perini
				// http://javascript.nwbox.com/IEContentLoaded/
				document.documentElement.doScroll("left")
			}catch(error){
				setTimeout( arguments.callee, 0)
				return;
			}
			//and execute any waiting functions
			functionref();
			dd_domreadycheck=true
		})();
	}
	if (document.attachEvent && parent.length>0) //account for page being in IFRAME, in which above doesn't fire in IE
		this.addEvent(window, function(){functionref()}, "load");
},


init:function(mainmenuid, dir){
	this.standardbody=(document.compatMode=="CSS1Compat")? document.documentElement : document.body
	this.topitemsindex=-1
	this.ulindex=-1
	this.topmenuids.push(mainmenuid)
	this.topitems[mainmenuid]=[] //declare array on object
	this.subuls[mainmenuid]=[] //declare array on object
	this.hidetimers[mainmenuid]=[] //declare hide entire menu timer
	if (this.enableshim && !this.shimadded){
		this.shimmy={}
		this.shimmy.topshim=this.addshimmy(document.body) //create top iframe shim obj
		this.shimmy.bottomshim=this.addshimmy(document.body) //create bottom iframe shim obj
		this.shimadded=true
	}
	var menubar=document.getElementById(mainmenuid)
	var alllinks=menubar.getElementsByTagName("a")
	this.getwindowsize()
	for (var i=0; i<alllinks.length; i++){
		if (alllinks[i].getAttribute('rel')){
			this.topitemsindex++
			this.ulindex++
			var menuitem=alllinks[i]
			this.topitems[mainmenuid][this.topitemsindex]=menuitem //store ref to main menu links
			var dropul=document.getElementById(menuitem.getAttribute('rel'))
			document.body.appendChild(dropul) //move main ULs to end of document
			dropul.style.zIndex=2000 //give drop down menus a high z-index
			dropul._master=mainmenuid  //Indicate which main menu this main UL is associated with
			dropul._pos=this.topitemsindex //Indicate which main menu item this main UL is associated with
			this.addEvent(dropul, function(){ddlevelsmenu.hidemenu(this)}, "click")
			var arrowclass=(dir=="sidebar")? "rightarrowpointer" : "downarrowpointer"
			var arrowpointer=(dir=="sidebar")? this.arrowpointers.rightarrow : this.arrowpointers.downarrow
			if (this.arrowpointers.showarrow.toplevel)
				this.addpointer(menuitem, arrowclass, arrowpointer, (dir=="sidebar")? "before" : "after")
			this.buildmenu(mainmenuid, menuitem, dropul, this.ulindex, true, dir) //build top level menu
			dropul.onmouseover=function(){
				clearTimeout(ddlevelsmenu.hidetimers[this._master][this._pos])
			}
			this.addEvent(dropul, function(e){ //hide menu if mouse moves out of main UL element into open space
				if (!ddlevelsmenu.isContained(this, e) && !ddlevelsmenu.isContained(ddlevelsmenu.topitems[this._master][parseInt(this._pos)], e)){
					var dropul=this
					if (ddlevelsmenu.enableshim)
						ddlevelsmenu.hideshim()
					ddlevelsmenu.hidetimers[this._master][this._pos]=setTimeout(function(){
						ddlevelsmenu.hidemenu(dropul)
					}, ddlevelsmenu.hideinterval)
				}
			}, "mouseout")
			var subuls=dropul.getElementsByTagName("ul")
			for (var c=0; c<subuls.length; c++){
				this.ulindex++
				var parentli=subuls[c].parentNode
				if (this.arrowpointers.showarrow.sublevel)
					this.addpointer(parentli.getElementsByTagName("a")[0], "rightarrowpointer", this.arrowpointers.rightarrow, "before")
				this.buildmenu(mainmenuid, parentli, subuls[c], this.ulindex, false, dir) //build sub level menus
			}
		}
	} //end for loop
	this.addEvent(window, function(){ddlevelsmenu.getwindowsize(); ddlevelsmenu.gettopitemsdimensions()}, "resize")
},

setup:function(mainmenuid, dir){
	this.domready(function(){ddlevelsmenu.init(mainmenuid, dir)})
}

}