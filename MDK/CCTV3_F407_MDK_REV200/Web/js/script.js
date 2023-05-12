
const pw = "uuddlrlrba";

$("#current_get_btn").bind("click", function () { Current_Get(this); });
$("#current_on_btn").bind("click", function () { Current_Set(this, true); });
$("#current_off_btn").bind("click", function () { Current_Set(this, false); });
$("#webhook_get_btn").bind("click", function () { Webhook_Get(this); });
$("#webhook_on_btn").bind("click", function () { Webhook_Set(this, true); });
$("#webhook_off_btn").bind("click", function () { Webhook_Set(this, false); });
$("#ip_get_btn").bind("click", function () { IP_Get(this); });
$("#ip_set_btn").bind("click", function () { IP_Set(this); });
$("#dhcp_get_btn").bind("click", function () { DHCP_Get(this); });
$("#dhcp_on_btn").bind("click", function () { DHCP_Set(this, true); });
$("#dhcp_off_btn").bind("click", function () { DHCP_Set(this, false); });
$("#sntp_get_btn").bind("click", function () { SNTP_Get(this); });
$("#sntp_on_btn").bind("click", function () { SNTP_Set(this, true); });
$("#sntp_off_btn").bind("click", function () { SNTP_Set(this, false); });
$("#camera_get_btn").bind("click", function () { Camera_Get(this); });
$("#camera_set_btn").bind("click", function () { Camera_Set(this); });
$("#camera_fps_btn").bind("click", function () { Camera_ShowFps(this); });
$("#light_get_btn").bind("click", function () { Light_Get(this); });
$("#light_set_btn").bind("click", function () { Light_Set(this); });
$("#reboot_btn").bind("click", function () { Reboot(this); });
$("#reset_btn").bind("click", function () { Reset(this); });

function Current_Get(sender) {
	var on_btn = $("#current_on_btn");
	var off_btn = $("#current_off_btn");
	var peak_threshold_text = $("#peak_threshold_text")[0];
	var peak_influence_text = $("#peak_influence_text")[0];
	var disconnect_thres_text = $("#disconnect_thres_text")[0];
	var overload_thres_text = $("#overload_thres_text")[0];

	axios.get('/api?cmd=current&act=get')
		.then(function (response) {
			ShowOnOff_Button(on_btn, off_btn, response.data.enable);
			peak_threshold_text.value = response.data.threshold;
			peak_influence_text.value = response.data.influence;
			disconnect_thres_text.value = response.data.disconnect;
			overload_thres_text.value = response.data.overload;
			ShowOk_jQuery(sender, 2000);
		})
		.catch(function (error) {
			Shake_jQuery(sender, 500);
			console.log(error);
		}).finally(function () {
			sender.blur();
		});
}

function Current_Set(sender, en) {
	var on_btn = $("#current_on_btn");
	var off_btn = $("#current_off_btn");
	var peak_threshold_text = $("#peak_threshold_text");
	var peak_influence_text = $("#peak_influence_text");
	var disconnect_thres_text = $("#disconnect_thres_text");
	var overload_thres_text = $("#overload_thres_text");
	var enable = en ? "true" : "false";

	axios.get('/api?cmd=current&act=set&enable=' + enable +
		'&disconnect=' + disconnect_thres_text.val() +
		'&overload=' + overload_thres_text.val() +
		'&threshold' + peak_threshold_text.val() +
		'&influence=' + peak_influence_text.val()
	)
		.then(function (response) {
			ShowOnOff_Button(on_btn, off_btn, en);
			ShowOk_jQuery(sender, 2000);
		})
		.catch(function (error) {
			console.log(error);
			Shake_jQuery(sender, 500);
		}).finally(function () {
			sender.blur();
		});
}

function Webhook_Get(sender) {
	var on_btn = $("#webhook_on_btn");
	var off_btn = $("#webhook_off_btn");
	var webhook_host_text = $("#webhook_host_text")[0];
	var webhook_uri_text = $("#webhook_uri_text")[0];
	var webhook_port_text = $("#webhook_port_text")[0];
	var webhook_retry_text = $("#webhook_retry_text")[0];
	var webhook_delay_text = $("#webhook_delay_text")[0];

	axios.get('/api?cmd=webhook&act=get')
		.then(function (response) {
			ShowOnOff_Button(on_btn, off_btn, response.data.enable);
			webhook_host_text.value = response.data.host;
			webhook_uri_text.value = response.data.uri;
			webhook_port_text.value = response.data.port;
			webhook_retry_text.value = response.data.retrys;
			webhook_delay_text.value = response.data.retry_delay;
			ShowOk_jQuery(sender, 2000);
		})
		.catch(function (error) {
			Shake_jQuery(sender, 500);
			console.log(error);
		}).finally(function () {
			sender.blur();
		});
}

function Webhook_Set(sender, en) {
	var on_btn = $("#webhook_on_btn");
	var off_btn = $("#webhook_off_btn");
	var webhook_host_text = $("#webhook_host_text");
	var webhook_uri_text = $("#webhook_uri_text");
	var webhook_port_text = $("#webhook_port_text");
	var webhook_retry_text = $("#webhook_retry_text");
	var webhook_delay_text = $("#webhook_delay_text");
	var enable = en ? 'true' : 'false';

	axios.get('/api?cmd=webhook&act=set&enable=' + enable +
		'&retrys=' + webhook_retry_text.val() +
		'&retry_delay=' + webhook_delay_text.val() +
		'&host=' + webhook_host_text.val() +
		'&uri=' + webhook_uri_text.val() +
		'&port=' + webhook_port_text.val()
	)
		.then(function (response) {
			ShowOnOff_Button(on_btn, off_btn, en);
			ShowOk_jQuery(sender, 2000);
		})
		.catch(function (error) {
			console.log(error);
			Shake_jQuery(sender, 500);
		}).finally(function () {
			sender.blur();
		});
}

function IP_Get(sender) {
	var ipaddr = $("#ipaddr_text")[0];
	var netmask = $("#netmask_text")[0];
	var gateway = $("#gateway_text")[0];
	var dns0 = $("#dns0_text")[0];
	var dns1 = $("#dns1_text")[0];

	axios.get('/api?cmd=ip&act=get')
		.then(function (response) {
			ipaddr.value = response.data.ip;
			netmask.value = response.data.netmask;
			gateway.value = response.data.gateway;
			dns0.value = response.data.dns0;
			dns1.value = response.data.dns1;
			ShowOk_jQuery(sender, 2000);
		})
		.catch(function (error) {
			Shake_jQuery(sender, 500);
			console.log(error);
		}).finally(function () {
			sender.blur();
		});
}

function IP_Set(sender) {
	var ipaddr = $("#ipaddr_text");
	var netmask = $("#netmask_text");
	var gateway = $("#gateway_text");
	var dns0 = $("#dns0_text");
	var dns1 = $("#dns1_text");

	if (!ipaddr.val() || !ValidateIPaddress(ipaddr.val())) {
		Shake_jQuery(ipaddr, 500);
		Highlight_jQuery(ipaddr, 500);
	}
	else if (!netmask.val() || !ValidateIPaddress(netmask.val())) {
		Shake_jQuery(netmask, 500);
		Highlight_jQuery(netmask, 500);
	}
	else if (!gateway.val() || !ValidateIPaddress(gateway.val())) {
		Shake_jQuery(gateway, 500);
		Highlight_jQuery(gateway, 500);
	}
	else if (!dns0.val() || !ValidateIPaddress(dns0.val())) {
		Shake_jQuery(dns0, 500);
		Highlight_jQuery(dns0, 500);
	}
	else if (!dns1.val() || !ValidateIPaddress(dns1.val())) {
		Shake_jQuery(dns1, 500);
		Highlight_jQuery(dns1, 500);
	}
	else {
		axios.get('/api?cmd=ip&act=set&ip=' + ipaddr.val() +
			'&mask=' + netmask.val() +
			'&gw=' + gateway.val() +
			'&dns0=' + dns0.val() +
			'&dns1=' + dns1.val()
		)
			.then(function (response) {
				DHCP_Get($("#dhcp_get_btn")[0]);
				ShowOk_jQuery(sender, 2000);
			})
			.catch(function (error) {
				console.log(error);
				Shake_jQuery(sender, 500);
			}).finally(function () {
				sender.blur();
			});
	}
}

function DHCP_Get(sender) {
	const on_btn = $("#dhcp_on_btn");
	const off_btn = $("#dhcp_off_btn");

	axios.get('/api?&cmd=dhcp&act=get')
		.then(function (response) {
			ShowOnOff_Button(on_btn, off_btn, response.data.DHCP);
			ShowOk_jQuery(sender, 2000);
		}).catch(function (error) {
			console.log(error);
			Shake_jQuery(sender, 500);
		}).finally(function () {
			sender.blur();
		});
}

function DHCP_Set(sender, en)
{
	const on_btn = $("#dhcp_on_btn");
	const off_btn = $("#dhcp_off_btn");
	var enable = en ? 'true' : 'false';

	axios.get('/api?cmd=dhcp&act=set&value=' + enable)
		.then(function (response) {
			if (response.data.result) {
				ShowOnOff_Button(on_btn, off_btn, en);
				ShowOk_jQuery(sender, 2000);
			}
			else
				throw new Error('DHCP_Set: ' + response.data.result);
		})
		.catch(function (error) {
			console.log(error);
			Shake_jQuery(sender, 500);
		}).finally(function () {
			sender.blur();
		});
}

function SNTP_Get(sender) {
	const on_btn = $("#sntp_on_btn");
	const off_btn = $("#sntp_off_btn");

	var sntp_text = $("#sntp_text")[0];

	axios.get('/api?cmd=sntp&act=get')
		.then(function (response) {
			ShowOnOff_Button(on_btn, off_btn, response.data.enable);
			sntp_text.value = response.data.server;
			ShowOk_jQuery(sender, 2000);
		})
		.catch(function (error) {
			console.log(error);
			Shake_jQuery(sender, 500);
		}).finally(function () {
			sender.blur();
		});
}

function SNTP_Set(sender, en) {
	var on_btn = $("#sntp_on_btn");
	var off_btn = $("#sntp_off_btn");
	var sntp_text = $("#sntp_text");
	var enable = en ? 'true' : 'false';

	axios.get('/api?cmd=sntp&act=set&enable=' + enable +
		'&server=' + sntp_text.val()
	)
		.then(function (response) {
			ShowOnOff_Button(on_btn, off_btn, en);
			ShowOk_jQuery(sender, 2000);
		})
		.catch(function (error) {
			console.log(error);
			Shake_jQuery(sender, 500);
		}).finally(function () {
			sender.blur();
		});
}

function Camera_Get(sender) {
	var qs_text = $("#qs_text")[0];
	var brightness_text = $("#brightness_text")[0];
	var contrast_text = $("#contrast_text")[0];
	var flip_checkbox = $("#flip_checkbox")[0];
	var mirror_checkbox = $("#mirror_checkbox")[0];

	axios.get('/api?cmd=camera&act=get')
		.then(function (response) {
			qs_text.value = response.data.qs;
			brightness_text.value = response.data.brightness;
			contrast_text.value = response.data.contrast;
			flip_checkbox.checked = response.data.flip;
			mirror_checkbox.checked = response.data.mirror;
			ShowOk_jQuery(sender, 2000);
		})
		.catch(function (error) {
			console.log(error);
			Shake_jQuery(sender, 500);
		}).finally(function () {
			sender.blur();
		});
}

function Camera_Set(sender) {
	var qs_text = $("#qs_text");
	var brightness_text = $("#brightness_text");
	var contrast_text = $("#contrast_text");
	var flip_checkbox = $("#flip_checkbox");
	var mirror_checkbox = $("#mirror_checkbox");

	var flip = flip_checkbox.is(":checked") ? 'true' : 'false';
	var mirror = mirror_checkbox.is(":checked") ? 'true' : 'false';

	axios.get('/api?cmd=camera&act=set' +
		'&qs=' + qs_text.val() +
		'&brightness=' + brightness_text.val() +
		'&contrast=' + contrast_text.val() +
		'&flip=' + flip +
		'&mirror=' + mirror
	)
		.then(function (response) {
			ShowOk_jQuery(sender, 2000);
		})
		.catch(function (error) {
			console.log(error);
			Shake_jQuery(sender, 500);
		}).finally(function () {
			sender.blur();
		});
}

function Camera_ShowFps(sender) {
	window.open("http://" + window.location.host + ":8080/view/fps", '_blank').focus();
}

function Light_Get(sender) {
	var light_top_text = $("#light_top_text")[0];
	var light_bottom_text = $("#light_bottom_text")[0];

	axios.all([
		axios.get('/api?cmd=light&act=get&pos=top'),
		axios.get('/api?cmd=light&act=get&pos=bottom')
	])
	.then(axios.spread((response1, response2) => {
		light_top_text.value = response1.data.value;
		light_bottom_text.value = response2.data.value;
		ShowOk_jQuery(sender, 2000);
	}))
	.catch(function (error) {
		console.log(error);
		Shake_jQuery(sender, 500);
	}).finally(function () {
		sender.blur();
	});
}

function Light_Set(sender) {

	var light_top_text = $("#light_top_text");
	var light_bottom_text = $("#light_bottom_text");

	axios.all([
		axios.get('/api?cmd=light&act=set&pos=top&value='+light_top_text.val()),
		axios.get('/api?cmd=light&act=set&pos=bottom&value='+light_bottom_text.val())
	])
	.then(axios.spread((response1, response2) => {
		if(response1.data.result && response2.data.result)
			ShowOk_jQuery(sender, 2000);
	})).catch(function (error) {
		console.log(error);
		Shake_jQuery(sender, 500);
	}).finally(function () {
		sender.blur();
	});
}

function Reboot(sender) {
	const ipaddr_text = $("#ipaddr_text");

	axios.get('/api?pw=' + pw + '&cmd=reboot')
		.then(function (response) {
			ShowOk_jQuery(sender, 2000);
			var new_ip = ipaddr_text.val();

			setTimeout(function () {
				if (new_ip && window.location.host !== new_ip)
					location.href = 'http://' + new_ip;
			}, 5000);
		})
		.catch(function (error) {
			Shake_jQuery(sender, 500);
			console.log(error);
		}).finally(function () {
			sender.blur();
		});
}

function Reset(sender) {
	axios.get('/api?pw=' + pw + '&cmd=reset')
		.then(function (response) {
			ShowOk_jQuery(sender, 2000);
		})
		.catch(function (error) {
			Shake_jQuery(sender, 500);
			console.log(error);
		}).finally(function () {
			sender.blur();
		});
}


function UpdateUptimeLoop() {
	axios.get('/api?cmd=uptime')
		.then(function (response) {
			var UpTime_Dom = $('#uptime')[0];
			var NowTime_Dom = $('#nowtime')[0];

			UpTime_Dom.innerText = convertTick(response.data.uptime);
			NowTime_Dom.innerText = convertDate(response.data.nowtime);
		})
		.catch(function (error) {
			console.log(error);
		});
}

function ShowOk_jQuery(obj, time) {
	var dom = obj;
	if (obj instanceof jQuery)
		dom = obj[0];

	const class_name = 'ok_active_color';
	var text = dom.innerText;
	var remove = !dom.classList.contains(class_name);

	dom.innerText = "OK";
	dom.classList.add(class_name);
	setTimeout(function () {
		dom.innerText = text;
		if (remove)
			dom.classList.remove(class_name);
	}, time);
}

function Shake_jQuery(obj, time) {
	var dom = obj;
	if (obj instanceof jQuery)
		dom = obj[0];

	dom.classList.add('shake');
	setTimeout(function () { dom.classList.remove('shake'); }, time);
}

function Highlight_jQuery(obj, time) {
	var dom = obj;
	if (obj instanceof jQuery)
		dom = obj[0];

	dom.classList.add('highlight_red');
	setTimeout(function () { dom.classList.remove('highlight_red'); }, time);
}

function ValidateIPaddress(ipaddress) {
	if (/^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(ipaddress)) {
		return (true)
	}
	return (false)
}

function ShowOnOff_Button(on_btn, off_btn, en) {
	const class_name = 'ok_active_color';
	if (en) {
		on_btn.addClass(class_name);
		off_btn.removeClass(class_name);
	}
	else {
		on_btn.removeClass(class_name);
		off_btn.addClass(class_name);
	}
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
	return d + ' D   ' + h + ' H   ' + m + ' M   ' + s + ' S   ';
}

function convertDate(unix_timestamp) {
	var date = new Date(unix_timestamp * 1000);
	//date.setHours(date.getHours()-((new Date()).getTimezoneOffset()/60));
	var y, M, d, h, m, s;

	y = date.getFullYear();
	M = '0' + (date.getMonth() + 1);
	d = '0' + date.getDate();

	h = '0' + date.getHours();
	m = '0' + date.getMinutes();
	s = '0' + date.getSeconds();

	return y + ' - ' + M + ' - ' + d.slice(-2) + ' ' + h.slice(-2) + ' : ' + m.slice(-2) + ' : ' + s.slice(-2);
}

var Systime_Update_Timer;

function PageChanged() {
	var mjpeg_img = $("#mjpeg_img");
	if (mjpeg_img.is(":visible"))
		mjpeg_img[0].src = "http://" + window.location.host + ":8080/stream";
	else
		mjpeg_img[0].src = "";

	var reboot_block = $("#reboot_block");
	if (reboot_block.is(":visible"))
		Systime_Update_Timer = setInterval(UpdateUptimeLoop, 1000);
	else
		clearInterval(Systime_Update_Timer);
}


