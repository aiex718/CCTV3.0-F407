

$( "#ip_get_btn" ).bind("click", function(){IP_Get(true);});

$( "#ip_set_btn" ).bind("click", function() {
	var ipaddr = $("#ipaddr_text");
	var netmask = $("#netmask_text");
	var gateway = $("#gateway_text");
	
	if(!ipaddr.val() || !ValidateIPaddress(ipaddr.val()))
	{
		Shake_jQuery(ipaddr,500);
		Highlight_jQuery(ipaddr,500);
	}
	else if(!netmask.val() || !ValidateIPaddress(netmask.val()))
	{
		Shake_jQuery(netmask,500);
		Highlight_jQuery(netmask,500);
	}
	else if(!gateway.val() || !ValidateIPaddress(gateway.val()))
	{
		Shake_jQuery(gateway,500);
		Highlight_jQuery(gateway,500);
	}
	else
	{
		axios.get('/api?username='+uname+'&pw='+pw+'&cmd=ip&act=set&ip='+ ipaddr.val() +'&mask='+ netmask.val() +'&gw='+gateway.val())
		.then(function (response) {
			ShowOk_jQuery($("#ip_set_btn"),2000);
			DHCP_Get();
		})
		.catch(function (error) {		
			console.log(error);
			Shake_jQuery($("#ip_set_btn"),500);
		}).finally(function(){
			$("#ip_set_btn").blur();
		});
	}
});

$( "#dhcp_on_btn" ).bind("click", function() {
	DHCP_Set(true);
});

$( "#dhcp_off_btn" ).bind("click", function() {
	DHCP_Set(false);
});

$( "#reboot_btn" ).bind("click", function() {
	
	axios.get('/api?username='+uname+'&pw='+pw+'&cmd=reboot')
	.then(function (response) {
		ShowOk_jQuery($("#reboot_btn"),2000);
		var new_ip = $('#ipaddr_text').val();
		
		setTimeout(function(){
			if(new_ip && window.location.host!==new_ip)
				location.href = 'http://'+new_ip;
		},8000);
	})
	.catch(function (error) {		
		Shake_jQuery($("#reboot_btn"),500);
		console.log(error);
	}).finally(function(){
		$("#reboot_btn").blur();
	});
});

$( "#format_btn" ).bind("click", function() {
	
	if (confirm('Confirm format disk?')) 
	{
		axios.get('/api?username='+uname+'&pw='+pw+'&cmd=formatdisk')
		.then(function (response) {
			if(response.data.FR_Result==0)//success,device will reboot
			{
				ShowOk_jQuery($("#format_btn"),2000);
			}
			else
			{
				throw new Error("Format failed,FR_Result:"+response.data.FR_Result);
			}
		})
		.catch(function (error) {		
			Shake_jQuery($("#format_btn"),500);
			console.log(error);
		}).finally(function(){
			$("#format_btn").blur();
		});
	}
	else
		$("#format_btn").blur();
	
});

function IP_Get(notify) {
	
	var ipaddr = $("#ipaddr_text")[0];
	var netmask = $("#netmask_text")[0];
	var gateway = $("#gateway_text")[0];
		
	axios.get('/api?username='+uname+'&pw='+pw+'&cmd=ip&act=get')
	.then(function (response) {
		if(notify)
			ShowOk_jQuery($("#ip_get_btn"),2000);
		ipaddr.value = response.data.ip;
		netmask.value = response.data.netmask;
		gateway.value = response.data.gateway;		
	})
	.catch(function (error) {		
		if(notify)
			Shake_jQuery($("#ip_get_btn"),500);
		console.log(error);
	}).finally(function(){
		if(notify)
			$("#ip_get_btn").blur();
	});
}

function DHCP_Get()
{
	axios.get('/api?username='+uname+'&pw='+pw+'&cmd=dhcp&act=get')
	.then(function (response) {
		if(response.data.DHCP)
		{
			$('#dhcp_on_btn').addClass('ok_active_color');
			$('#dhcp_off_btn').removeClass('ok_active_color');
		}
		else
		{
			$('#dhcp_on_btn').removeClass('ok_active_color');
			$('#dhcp_off_btn').addClass('ok_active_color');
		}
	}).catch(function (error) {		
		console.log(error);
		Shake_jQuery($("#DHCP_text"),500);
	}).finally(function(){
		$("#dhcp_on_btn").blur();
		$("#dhcp_off_btn").blur();
	});
}

function DHCP_Set(val)
{
	axios.get('/api?username='+uname+'&pw='+pw+'&cmd=dhcp&act=set&value='+ (val ? 'true':'false'))
	.then(function (response) {
		if(response.data.DHCP)
		{
			$('#dhcp_on_btn').addClass('ok_active_color');
			$('#dhcp_off_btn').removeClass('ok_active_color');
		}
		else
		{
			$('#dhcp_on_btn').removeClass('ok_active_color');
			$('#dhcp_off_btn').addClass('ok_active_color');
		}
	})
	.catch(function (error) {		
		console.log(error);
		if(val)		
			Shake_jQuery($("#dhcp_on_btn"),500);
		else
			Shake_jQuery($("#dhcp_off_btn"),500);
	}).finally(function(){
		$("#dhcp_on_btn").blur();
		$("#dhcp_off_btn").blur();
	});
}

var UpTime_UpdateTimeout;
function UpdateUptimeLoop()
{
	axios.get('/api?cmd=uptime')
	.then(function (response) {	
		var UpTime_Dom = $('#uptime')[0];
		var NowTime_Dom = $('#nowtime')[0];
		
		UpTime_Dom.innerText=convertTick(response.data.uptime);
		NowTime_Dom.innerText=convertDate(response.data.nowtime);
		setTimeout(UpdateUptimeLoop,1000);
	})
	.catch(function (error) {		
		console.log(error);
	});
}

function ShowOk_jQuery(obj,time)
{
	var text = obj[0].innerText;
	obj[0].innerText = "OK";
	obj.addClass('ok_active_color'); 
	setTimeout(function(){
		obj[0].innerText=text; 
		obj.removeClass('ok_active_color'); 
	},time);
}

function Shake_jQuery(obj,time)
{
	obj.addClass('shake'); 
	setTimeout(function(){obj.removeClass('shake'); },time);
}

function Highlight_jQuery(dom,time)
{
	dom.addClass('highlight_red'); 
	setTimeout(function(){dom.removeClass('highlight_red'); },time);
}

function ValidateIPaddress(ipaddress) 
{
	if (/^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(ipaddress)) {  
		return (true)  
	}  
	return (false)
}

function convertTick(ms) {
    var d, h, m, s;
    s = Math.floor(ms / 1000);
    m = Math.floor(s / 60);
    s = s % 60;
    h = Math.floor(m / 60);
    m = m % 60;
    d = Math.floor(h / 24);
    h = h % 24;
    h += d * 24;
    return d + ' D   '+ h + ' H   ' + m + ' M   ' + s +' S   ';
}

function convertDate(unix_timestamp) {
    var date = new Date(unix_timestamp*1000);
	//date.setHours(date.getHours()-((new Date()).getTimezoneOffset()/60));
	var y, M, d, h , m , s ;
	
    y=date.getFullYear();
	M='0'+(date.getMonth()+1);
	d='0'+date.getDate();
	
	h='0'+date.getHours();
	m='0'+date.getMinutes();
	s='0'+date.getSeconds();
	
    return y +' - '+ M + ' - '+ d.slice(-2) + ' '+ h.slice(-2) + ' : ' + m.slice(-2) + ' : ' + s.slice(-2) ;
}

$(function() {
	UpdateUptimeLoop();
	var mjpeg_img = $("#mjpeg_img")[0];
	mjpeg_img.src = "http://"+window.location.host+":8080/stream";
});

