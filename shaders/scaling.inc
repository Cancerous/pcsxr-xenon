<html><head><title>DOSBox</title>
<script language="javascript">
function popup(URL) {
	window.open(URL, 'archive', 'scrollbars=1,resizable=1,width=800,height=300');
}
</script>
</head><body BACKGROUND="back.gif">
<font size=5><b>DOSBox</b></font></a><hr>
<br><a href="http://www.DOSBox.com" target="_new">DOSBox</a> is an open source DOS-emulator. It runs on many different platforms (windows, linux, etc..) and although it's still in beta stage it is able to run a lot of older games which do not run on current systems.<br>
On this page you can get a recent CVS compile (a version that is newer than current release on the homepage, but as it is still in development it might be LESS stable) with some of the experimental patches.<br><br><hr>
NOTE: These builds are NOT supported by the DOSBox authors! If you have a problem they might or might not accept your bug report! Some of the features are still in development, they might not work on all systems! I do not accept any responsibility, the pre-compiled versions are available for your convenience only and will be updated when I feel like it :-)<hr><br>

<a name="Patches"><font size=4><b>Experimental patches</b></font></a><hr>
<br><a href="patch/dosbox_timesync.diff">timesync.diff</a> (obsolete) - This patch was sent to me by *orz* on the <a href="http://vogons.zetafleet.com" target="_new">Vogons</a> forum. It adds a new option in the dosbox.conf (you might have to add it manually) in the cpu section - timesynched=true/false. It allows the in-game time to be synchronized to actual time, which means you don't have to bother with cycles option anymore. Just set the cycles high enough (> 50K) and the game will never use too much CPU. This option can also be toggled on-the-fly with Control-= when DOSBox is running.
<br>
<br><a href="patch/dosbox_coreswitch.diff">coreswitch.diff</a> - A further modification suggested by *orz*. This patch allows to change CPU core while DOSBox is running! Pressing Control-1 switches to Normal core, Control-2 switches to Full core, Control-3 to Dynamic core and Control-4 switches to Simple core. Updated to support simple core (24.8.2004). Removed timesync code from this patch (18.3.2006), somewhat different 'autocycles' patch is now in official cvs.
<br>
<br><a href="patch/dosbox_direct3d.diff">direct3d.diff</a> - This patch will add a Direct3D output option for DOSBox. The goal is to have same speed and functionality as the OpenGL output without the need of NVidia extensions which are more or less required to get decent speed. This should be a recommended option for all ATi Radeon 8500 and up graphics cards. Other cards work as well, but if the drivers do not support dynamic textures (check the console output!) speed will be somewhat lower. It needs DirectX9. To compile first run autogen.sh. You will also need DirectX9 SDK. Latest MinGW already includes the files necessary to compile without shader support. For advanced shader scaling get official <a href="http://www.microsoft.com/windows/directx/default.aspx">Microsoft DirectX SDK</a> (version 9 or later). For MinGW, copy DxSDK include files to some MinGW reachable directory and convert d3dx9.lib (type 'reimp d3dx9.lib', reimp utility is part of mingw-utils package) so it can be used by MinGW. You can specify include and lib directories with configure switches (--with-dx-headers and --with-dx-libs). To use it, set output=direct3d in dosbox.conf. All display options work also, the only change from other display outputs is fulldouble option which now turns triple buffering on/off in Direct3D mode since DirectX always uses double buffering by default.<br>
<b>New options</b>: Direct3D output now supports the use of custom pixel shaders for picture scaling. To use it your card has to support PS3.0 (Radeon >= x1xxx Series, Geforce >= 7), PS2.0 (Radeon >=9500, GeForce >=FX5200) or PS1.4 for scale2x_ps14.fx (Radeon >= 8500).<br>
These shaders are currently available (the files have to be in Shaders subdirectory of the DOSBox dir, you need <a href="src/scaling.inc">scaling.inc</a> as well):<br>
<a href="src/point.fx">point.fx</a> - normal (nearest neighbor) resize<br>
<a href="src/bilinear.fx">bilinear.fx</a> - bilinear resize (slightly blurry)<br>
<a href="src/scale2x.fx">scale2x.fx</a> - advmame2x resize<br>
<a href="src/scale2x_ps14.fx">scale2x_ps14.fx</a> - advmame2x resize (PS1.4 version)<br>
<a href="src/2xSaI.fx">2xSaI.fx</a> - <a href="http://elektron.its.tudelft.nl/~dalikifa" target=new>2xSaI</a> resize<br>
<a href="src/2xSaI_sRGB.fx">2xSaI_sRGB.fx</a> - 2xSai (Gamma Corrected) resize<br>
<a href="src/hq2x.fx">hq2x.fx</a> - <a href="http://www.hiend3d.com/hq2x.html" target=new>HQ2x resize</a>, thanks to mtrooper for D3D port and
<a href="http://garni.ch/dosbox" target=new>Moe</a> for original OpenGL implementation</br><br>
The following are all shaders provided by guest(r) (updated 23.6.2007)<br>
Some of the shaders have settings you can modify by editing the .fx file.<br>
<a href="src/Shaders.zip">Shaders.zip</a> - Various different shaders for dosbox.<br><br>

<a href="gallery">Shader Gallery</a> - Screenshots of all available pixelshaders, thanks to Aybe<br><br>
Set 'pixelshader=' option in dosbox.conf to one of the shaders and 'scaler=hardware2x' or hardware3x (use normal2x in builds older than 18.3.2006). When pixel shader is active DOSBox will report it in the console:<br>
D3D:Pixel shader output enabled<br>
If you don't get this line then pixel shader effects are <b>not</b> in use.<br>
<br><br>

<a name="Download"><font size=4><b>Pre-compiled versions</b></font></a><hr>
<a href="dosbox/dosboxcvs-090704.zip">DOSBoxcvs-090704.zip</a> (04.07.2009) <a href="src/dosboxcvs-090704-src.tar.gz">source</a><br>
(with coreswitch, direct3d, glide, mt32, openglhq, physfs, NE2000, printer and oplpassthrough experimental patches)<br>
(requires d3dx9_30.dll, download <a href="bin/D3DX9_30_dll_update.zip">installer</a>)<br><br>

Click <a href="javascript:popup('archive.html')">here</a> for older releases.<br><br>

<br><a name="Links"><font size=4><b>Links</b></font></a><hr>
<a href="http://vogons.zetafleet.com" target="_new">Vogons DOSBox forum</a><br>
<a href="http://ykhwong.x-y.net" target="_new">DOSBox Daum Cafe</a> (ykhwong's page - also has compiled CVS version)<br>
<!--<a href="http://www.mypixels.dk/forums" target="_new">mypixels forum</a> (also has compiled CVS version)<br>--><br></body></html>
