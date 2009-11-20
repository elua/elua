


<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
 <script type="text/javascript">
 
 
 
 var codesite_token = null;
 
 
 var logged_in_user_email = null;
 
 </script>
 
 
 <title>snake.lua - 
 eluaexamples -
 
 Project Hosting on Google Code</title>
 <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" >
 
 <link type="text/css" rel="stylesheet" href="http://www.gstatic.com/codesite/ph/2429932258724909799/css/ph_core.css">
 
 <link type="text/css" rel="stylesheet" href="http://www.gstatic.com/codesite/ph/2429932258724909799/css/ph_detail.css" >
 
 
 <link type="text/css" rel="stylesheet" href="http://www.gstatic.com/codesite/ph/2429932258724909799/css/d_sb_20080522.css" >
 
 
 
<!--[if IE]>
 <link type="text/css" rel="stylesheet" href="http://www.gstatic.com/codesite/ph/2429932258724909799/css/d_ie.css" >
<![endif]-->
 <style type="text/css">
 .menuIcon.off { background: no-repeat url(http://www.gstatic.com/codesite/ph/images/dropdown_sprite.gif) 0 -42px }
 .menuIcon.on { background: no-repeat url(http://www.gstatic.com/codesite/ph/images/dropdown_sprite.gif) 0 -28px }
 .menuIcon.down { background: no-repeat url(http://www.gstatic.com/codesite/ph/images/dropdown_sprite.gif) 0 0; }
 .results th a, .results th a:visited { background: url(http://www.gstatic.com/codesite/ph/images/downarrow.gif) no-repeat top right }
 </style>
</head>
<body class="t4">
 <div id="gaia">
 
 <span>
 
 
 <a href="#" id="projects-dropdown" onclick="return false;">My favorites</a>
 
 | <a href="https://www.google.com/accounts/ServiceLogin?service=code&ltmpl=phosting&continue=http%3A%2F%2Fcode.google.com%2Fp%2Feluaexamples%2Fsource%2Fbrowse%2Ftrunk%2Fsnake%2Fsnake.lua&amp;followup=http%3A%2F%2Fcode.google.com%2Fp%2Feluaexamples%2Fsource%2Fbrowse%2Ftrunk%2Fsnake%2Fsnake.lua" onclick="_CS_click('/gb/ph/signin');">Sign in</a>
 
 </span>

 </div>
 <div class="gbh" style="left: 0pt;"></div>
 <div class="gbh" style="right: 0pt;"></div>
 
 
 <div style="height: 1px"></div>
<!--[if IE 6]>
<div style="text-align:center;">
Support browsers that contribute to open source, try <a href="http://www.firefox.com">Firefox</a> or <a href="http://www.google.com/chrome">Google Chrome</a>.
</div>
<![endif]-->
 <table style="padding:0px; margin: 20px 0px 0px 0px; width:100%" cellpadding="0" cellspacing="0">
 <tr style="height: 58px;">
 
 <td style="width: 55px; text-align:center;">
 <a href="/p/eluaexamples/">
 
 
 
 <img src="/p/eluaexamples/logo?logo_id=1254241294" alt="Logo">
 
 
 </a>
 </td>
 
 <td style="padding-left: 0.8em">
 
 <div id="pname" style="margin: 0px 0px -3px 0px">
 <a href="/p/eluaexamples/" style="text-decoration:none; color:#000">eluaexamples</a>
 </div>
 
 <div id="psum">
 <i><a id="project_summary_link" href="/p/eluaexamples/" style="text-decoration:none; color:#000">Code examples for the many platforms supported by eLua (www.eluaproject.net)</a></i>
 </div>
 
 </td>
 <td style="white-space:nowrap; text-align:right">
 
 <form action="/hosting/search">
 <input size="30" name="q" value="">
 <input type="submit" name="projectsearch" value="Search projects" >
 </form>
 
 </tr>
 </table>



 
<table id="mt" cellspacing="0" cellpadding="0" width="100%" border="0">
 <tr>
 <th onclick="if (!cancelBubble) _go('/p/eluaexamples/');">
 <div class="tab inactive">
 <div class="round4"></div>
 <div class="round2"></div>
 <div class="round1"></div>
 <div class="box-inner">
 <a onclick="cancelBubble=true;" href="/p/eluaexamples/">Project&nbsp;Home</a>
 </div>
 </div>
 </th><td>&nbsp;&nbsp;</td>
 
 
 
 
 <th onclick="if (!cancelBubble) _go('/p/eluaexamples/downloads/list');">
 <div class="tab inactive">
 <div class="round4"></div>
 <div class="round2"></div>
 <div class="round1"></div>
 <div class="box-inner">
 <a onclick="cancelBubble=true;" href="/p/eluaexamples/downloads/list">Downloads</a>
 </div>
 </div>
 </th><td>&nbsp;&nbsp;</td>
 
 
 
 
 
 <th onclick="if (!cancelBubble) _go('/p/eluaexamples/w/list');">
 <div class="tab inactive">
 <div class="round4"></div>
 <div class="round2"></div>
 <div class="round1"></div>
 <div class="box-inner">
 <a onclick="cancelBubble=true;" href="/p/eluaexamples/w/list">Wiki</a>
 </div>
 </div>
 </th><td>&nbsp;&nbsp;</td>
 
 
 
 
 
 <th onclick="if (!cancelBubble) _go('/p/eluaexamples/issues/list');">
 <div class="tab inactive">
 <div class="round4"></div>
 <div class="round2"></div>
 <div class="round1"></div>
 <div class="box-inner">
 <a onclick="cancelBubble=true;" href="/p/eluaexamples/issues/list">Issues</a>
 </div>
 </div>
 </th><td>&nbsp;&nbsp;</td>
 
 
 
 
 
 <th onclick="if (!cancelBubble) _go('/p/eluaexamples/source/checkout');">
 <div class="tab active">
 <div class="round4"></div>
 <div class="round2"></div>
 <div class="round1"></div>
 <div class="box-inner">
 <a onclick="cancelBubble=true;" href="/p/eluaexamples/source/checkout">Source</a>
 </div>
 </div>
 </th><td>&nbsp;&nbsp;</td>
 
 
 <td width="100%">&nbsp;</td>
 </tr>
</table>
<table cellspacing="0" cellpadding="0" width="100%" align="center" border="0" class="st">
 <tr>
 
 
 
 
 
 
 <td>
 <div class="st2">
 <div class="isf">
 
 
 
 <span class="inst1"><a href="/p/eluaexamples/source/checkout">Checkout</a></span> |
 <span class="inst2"><a href="/p/eluaexamples/source/browse/">Browse</a></span> |
 <span class="inst3"><a href="/p/eluaexamples/source/list">Changes</a></span> |
 
 <form action="http://www.google.com/codesearch" method="get" style="display:inline"
 onsubmit="document.getElementById('codesearchq').value = document.getElementById('origq').value + ' package:http://eluaexamples\\.googlecode\\.com'">
 <input type="hidden" name="q" id="codesearchq" value="">
 <input maxlength="2048" size="35" id="origq" name="origq" value="" title="Google Code Search" style="font-size:92%">&nbsp;<input type="submit" value="Search Trunk" name="btnG" style="font-size:92%">
 
 
 
 </form>
 </div>
</div>

 </td>
 
 
 
 
 
 <td height="4" align="right" valign="top" class="bevel-right">
 <div class="round4"></div>
 <div class="round2"></div>
 <div class="round1"></div>
 </td>
 </tr>
</table>
<script type="text/javascript">
 var cancelBubble = false;
 function _go(url) { document.location = url; }
</script>


<div id="maincol"
 
>

 
<!-- IE -->



<div class="expand">


<style type="text/css">
 #file_flipper { display: inline; float: right; white-space: nowrap; }
 #file_flipper.hidden { display: none; }
 #file_flipper .pagelink { color: #0000CC; text-decoration: underline; }
 #file_flipper #visiblefiles { padding-left: 0.5em; padding-right: 0.5em; }
</style>
<div id="nav_and_rev" class="heading">
 <div class="round4"></div>
 <div class="round2"></div>
 <div class="round1"></div>
 <div class="box-inner" id="bub">
 <div class="bub-top">
 <div class="pagination" style="margin-left: 2em">
 <table cellpadding="0" cellspacing="0" class="flipper">
 <tbody>
 <tr>
 
 <td><b>r28</b></td>
 
 </tr>
 </tbody>
 </table>
 </div>
 
 <div class="" style="vertical-align: top">
 <div class="src_crumbs src_nav">
 <strong class="src_nav">Source path:&nbsp;</strong>
 <span id="crumb_root">
 
 <a href="/p/eluaexamples/source/browse/">svn</a>/&nbsp;</span>
 <span id="crumb_links" class="ifClosed"><a href="/p/eluaexamples/source/browse/trunk/">trunk</a><span class="sp">/&nbsp;</span><a href="/p/eluaexamples/source/browse/trunk/snake/">snake</a><span class="sp">/&nbsp;</span>snake.lua</span>
 
 
 </div>
 
 </div>
 <div style="clear:both"></div>
 </div>
 </div>
</div>

<style type="text/css">
 
  tr.inline_comment {
 background: #fff;
 vertical-align: top;
 }
 div.draft, div.published {
 padding: .3em;
 border: 1px solid #999; 
 margin-bottom: .1em;
 font-family: arial, sans-serif;
 max-width: 60em;
 }
 div.draft {
 background: #ffa;
 } 
 div.published {
 background: #e5ecf9;
 }
 div.published .body, div.draft .body {
 padding: .5em .1em .1em .1em;
 max-width: 60em;
 white-space: pre-wrap;
 white-space: -moz-pre-wrap;
 white-space: -pre-wrap;
 white-space: -o-pre-wrap;
 word-wrap: break-word;
 }
 div.draft .actions {
 margin-left: 1em;
 font-size: 90%;
 }
 div.draft form {
 padding: .5em .5em .5em 0;
 }
 div.draft textarea, div.published textarea {
 width: 95%;
 height: 10em;
 font-family: arial, sans-serif;
 margin-bottom: .5em;
 }


 
 .nocursor, .nocursor td, .cursor_hidden, .cursor_hidden td {
 background-color: white;
 height: 2px;
 }
 .cursor, .cursor td {
 background-color: darkblue;
 height: 2px;
 display: '';
 }

</style>
<div class="fc">
 


<table class="opened" id="review_comment_area"><tr>
<td id="nums">
<pre><table width="100%"><tr class="nocursor"><td></td></tr></table></pre>

<pre><table width="100%"><tr id="gr_svn28_1"><td id="1"><a href="#1">1</a></td></tr
><tr id="gr_svn28_2"><td id="2"><a href="#2">2</a></td></tr
><tr id="gr_svn28_3"><td id="3"><a href="#3">3</a></td></tr
><tr id="gr_svn28_4"><td id="4"><a href="#4">4</a></td></tr
><tr id="gr_svn28_5"><td id="5"><a href="#5">5</a></td></tr
><tr id="gr_svn28_6"><td id="6"><a href="#6">6</a></td></tr
><tr id="gr_svn28_7"><td id="7"><a href="#7">7</a></td></tr
><tr id="gr_svn28_8"><td id="8"><a href="#8">8</a></td></tr
><tr id="gr_svn28_9"><td id="9"><a href="#9">9</a></td></tr
><tr id="gr_svn28_10"><td id="10"><a href="#10">10</a></td></tr
><tr id="gr_svn28_11"><td id="11"><a href="#11">11</a></td></tr
><tr id="gr_svn28_12"><td id="12"><a href="#12">12</a></td></tr
><tr id="gr_svn28_13"><td id="13"><a href="#13">13</a></td></tr
><tr id="gr_svn28_14"><td id="14"><a href="#14">14</a></td></tr
><tr id="gr_svn28_15"><td id="15"><a href="#15">15</a></td></tr
><tr id="gr_svn28_16"><td id="16"><a href="#16">16</a></td></tr
><tr id="gr_svn28_17"><td id="17"><a href="#17">17</a></td></tr
><tr id="gr_svn28_18"><td id="18"><a href="#18">18</a></td></tr
><tr id="gr_svn28_19"><td id="19"><a href="#19">19</a></td></tr
><tr id="gr_svn28_20"><td id="20"><a href="#20">20</a></td></tr
><tr id="gr_svn28_21"><td id="21"><a href="#21">21</a></td></tr
><tr id="gr_svn28_22"><td id="22"><a href="#22">22</a></td></tr
><tr id="gr_svn28_23"><td id="23"><a href="#23">23</a></td></tr
><tr id="gr_svn28_24"><td id="24"><a href="#24">24</a></td></tr
><tr id="gr_svn28_25"><td id="25"><a href="#25">25</a></td></tr
><tr id="gr_svn28_26"><td id="26"><a href="#26">26</a></td></tr
><tr id="gr_svn28_27"><td id="27"><a href="#27">27</a></td></tr
><tr id="gr_svn28_28"><td id="28"><a href="#28">28</a></td></tr
><tr id="gr_svn28_29"><td id="29"><a href="#29">29</a></td></tr
><tr id="gr_svn28_30"><td id="30"><a href="#30">30</a></td></tr
><tr id="gr_svn28_31"><td id="31"><a href="#31">31</a></td></tr
><tr id="gr_svn28_32"><td id="32"><a href="#32">32</a></td></tr
><tr id="gr_svn28_33"><td id="33"><a href="#33">33</a></td></tr
><tr id="gr_svn28_34"><td id="34"><a href="#34">34</a></td></tr
><tr id="gr_svn28_35"><td id="35"><a href="#35">35</a></td></tr
><tr id="gr_svn28_36"><td id="36"><a href="#36">36</a></td></tr
><tr id="gr_svn28_37"><td id="37"><a href="#37">37</a></td></tr
><tr id="gr_svn28_38"><td id="38"><a href="#38">38</a></td></tr
><tr id="gr_svn28_39"><td id="39"><a href="#39">39</a></td></tr
><tr id="gr_svn28_40"><td id="40"><a href="#40">40</a></td></tr
><tr id="gr_svn28_41"><td id="41"><a href="#41">41</a></td></tr
><tr id="gr_svn28_42"><td id="42"><a href="#42">42</a></td></tr
><tr id="gr_svn28_43"><td id="43"><a href="#43">43</a></td></tr
><tr id="gr_svn28_44"><td id="44"><a href="#44">44</a></td></tr
><tr id="gr_svn28_45"><td id="45"><a href="#45">45</a></td></tr
><tr id="gr_svn28_46"><td id="46"><a href="#46">46</a></td></tr
><tr id="gr_svn28_47"><td id="47"><a href="#47">47</a></td></tr
><tr id="gr_svn28_48"><td id="48"><a href="#48">48</a></td></tr
><tr id="gr_svn28_49"><td id="49"><a href="#49">49</a></td></tr
><tr id="gr_svn28_50"><td id="50"><a href="#50">50</a></td></tr
><tr id="gr_svn28_51"><td id="51"><a href="#51">51</a></td></tr
><tr id="gr_svn28_52"><td id="52"><a href="#52">52</a></td></tr
><tr id="gr_svn28_53"><td id="53"><a href="#53">53</a></td></tr
><tr id="gr_svn28_54"><td id="54"><a href="#54">54</a></td></tr
><tr id="gr_svn28_55"><td id="55"><a href="#55">55</a></td></tr
><tr id="gr_svn28_56"><td id="56"><a href="#56">56</a></td></tr
><tr id="gr_svn28_57"><td id="57"><a href="#57">57</a></td></tr
><tr id="gr_svn28_58"><td id="58"><a href="#58">58</a></td></tr
><tr id="gr_svn28_59"><td id="59"><a href="#59">59</a></td></tr
><tr id="gr_svn28_60"><td id="60"><a href="#60">60</a></td></tr
><tr id="gr_svn28_61"><td id="61"><a href="#61">61</a></td></tr
><tr id="gr_svn28_62"><td id="62"><a href="#62">62</a></td></tr
><tr id="gr_svn28_63"><td id="63"><a href="#63">63</a></td></tr
><tr id="gr_svn28_64"><td id="64"><a href="#64">64</a></td></tr
><tr id="gr_svn28_65"><td id="65"><a href="#65">65</a></td></tr
><tr id="gr_svn28_66"><td id="66"><a href="#66">66</a></td></tr
><tr id="gr_svn28_67"><td id="67"><a href="#67">67</a></td></tr
><tr id="gr_svn28_68"><td id="68"><a href="#68">68</a></td></tr
><tr id="gr_svn28_69"><td id="69"><a href="#69">69</a></td></tr
><tr id="gr_svn28_70"><td id="70"><a href="#70">70</a></td></tr
><tr id="gr_svn28_71"><td id="71"><a href="#71">71</a></td></tr
><tr id="gr_svn28_72"><td id="72"><a href="#72">72</a></td></tr
><tr id="gr_svn28_73"><td id="73"><a href="#73">73</a></td></tr
><tr id="gr_svn28_74"><td id="74"><a href="#74">74</a></td></tr
><tr id="gr_svn28_75"><td id="75"><a href="#75">75</a></td></tr
><tr id="gr_svn28_76"><td id="76"><a href="#76">76</a></td></tr
><tr id="gr_svn28_77"><td id="77"><a href="#77">77</a></td></tr
><tr id="gr_svn28_78"><td id="78"><a href="#78">78</a></td></tr
><tr id="gr_svn28_79"><td id="79"><a href="#79">79</a></td></tr
><tr id="gr_svn28_80"><td id="80"><a href="#80">80</a></td></tr
><tr id="gr_svn28_81"><td id="81"><a href="#81">81</a></td></tr
><tr id="gr_svn28_82"><td id="82"><a href="#82">82</a></td></tr
><tr id="gr_svn28_83"><td id="83"><a href="#83">83</a></td></tr
><tr id="gr_svn28_84"><td id="84"><a href="#84">84</a></td></tr
><tr id="gr_svn28_85"><td id="85"><a href="#85">85</a></td></tr
><tr id="gr_svn28_86"><td id="86"><a href="#86">86</a></td></tr
><tr id="gr_svn28_87"><td id="87"><a href="#87">87</a></td></tr
><tr id="gr_svn28_88"><td id="88"><a href="#88">88</a></td></tr
><tr id="gr_svn28_89"><td id="89"><a href="#89">89</a></td></tr
><tr id="gr_svn28_90"><td id="90"><a href="#90">90</a></td></tr
><tr id="gr_svn28_91"><td id="91"><a href="#91">91</a></td></tr
><tr id="gr_svn28_92"><td id="92"><a href="#92">92</a></td></tr
><tr id="gr_svn28_93"><td id="93"><a href="#93">93</a></td></tr
><tr id="gr_svn28_94"><td id="94"><a href="#94">94</a></td></tr
><tr id="gr_svn28_95"><td id="95"><a href="#95">95</a></td></tr
><tr id="gr_svn28_96"><td id="96"><a href="#96">96</a></td></tr
><tr id="gr_svn28_97"><td id="97"><a href="#97">97</a></td></tr
><tr id="gr_svn28_98"><td id="98"><a href="#98">98</a></td></tr
><tr id="gr_svn28_99"><td id="99"><a href="#99">99</a></td></tr
><tr id="gr_svn28_100"><td id="100"><a href="#100">100</a></td></tr
><tr id="gr_svn28_101"><td id="101"><a href="#101">101</a></td></tr
><tr id="gr_svn28_102"><td id="102"><a href="#102">102</a></td></tr
><tr id="gr_svn28_103"><td id="103"><a href="#103">103</a></td></tr
><tr id="gr_svn28_104"><td id="104"><a href="#104">104</a></td></tr
><tr id="gr_svn28_105"><td id="105"><a href="#105">105</a></td></tr
><tr id="gr_svn28_106"><td id="106"><a href="#106">106</a></td></tr
><tr id="gr_svn28_107"><td id="107"><a href="#107">107</a></td></tr
><tr id="gr_svn28_108"><td id="108"><a href="#108">108</a></td></tr
><tr id="gr_svn28_109"><td id="109"><a href="#109">109</a></td></tr
><tr id="gr_svn28_110"><td id="110"><a href="#110">110</a></td></tr
><tr id="gr_svn28_111"><td id="111"><a href="#111">111</a></td></tr
><tr id="gr_svn28_112"><td id="112"><a href="#112">112</a></td></tr
><tr id="gr_svn28_113"><td id="113"><a href="#113">113</a></td></tr
><tr id="gr_svn28_114"><td id="114"><a href="#114">114</a></td></tr
><tr id="gr_svn28_115"><td id="115"><a href="#115">115</a></td></tr
><tr id="gr_svn28_116"><td id="116"><a href="#116">116</a></td></tr
><tr id="gr_svn28_117"><td id="117"><a href="#117">117</a></td></tr
><tr id="gr_svn28_118"><td id="118"><a href="#118">118</a></td></tr
><tr id="gr_svn28_119"><td id="119"><a href="#119">119</a></td></tr
><tr id="gr_svn28_120"><td id="120"><a href="#120">120</a></td></tr
><tr id="gr_svn28_121"><td id="121"><a href="#121">121</a></td></tr
><tr id="gr_svn28_122"><td id="122"><a href="#122">122</a></td></tr
><tr id="gr_svn28_123"><td id="123"><a href="#123">123</a></td></tr
><tr id="gr_svn28_124"><td id="124"><a href="#124">124</a></td></tr
><tr id="gr_svn28_125"><td id="125"><a href="#125">125</a></td></tr
><tr id="gr_svn28_126"><td id="126"><a href="#126">126</a></td></tr
><tr id="gr_svn28_127"><td id="127"><a href="#127">127</a></td></tr
><tr id="gr_svn28_128"><td id="128"><a href="#128">128</a></td></tr
><tr id="gr_svn28_129"><td id="129"><a href="#129">129</a></td></tr
><tr id="gr_svn28_130"><td id="130"><a href="#130">130</a></td></tr
><tr id="gr_svn28_131"><td id="131"><a href="#131">131</a></td></tr
><tr id="gr_svn28_132"><td id="132"><a href="#132">132</a></td></tr
><tr id="gr_svn28_133"><td id="133"><a href="#133">133</a></td></tr
><tr id="gr_svn28_134"><td id="134"><a href="#134">134</a></td></tr
><tr id="gr_svn28_135"><td id="135"><a href="#135">135</a></td></tr
><tr id="gr_svn28_136"><td id="136"><a href="#136">136</a></td></tr
><tr id="gr_svn28_137"><td id="137"><a href="#137">137</a></td></tr
><tr id="gr_svn28_138"><td id="138"><a href="#138">138</a></td></tr
><tr id="gr_svn28_139"><td id="139"><a href="#139">139</a></td></tr
><tr id="gr_svn28_140"><td id="140"><a href="#140">140</a></td></tr
><tr id="gr_svn28_141"><td id="141"><a href="#141">141</a></td></tr
><tr id="gr_svn28_142"><td id="142"><a href="#142">142</a></td></tr
><tr id="gr_svn28_143"><td id="143"><a href="#143">143</a></td></tr
><tr id="gr_svn28_144"><td id="144"><a href="#144">144</a></td></tr
><tr id="gr_svn28_145"><td id="145"><a href="#145">145</a></td></tr
><tr id="gr_svn28_146"><td id="146"><a href="#146">146</a></td></tr
><tr id="gr_svn28_147"><td id="147"><a href="#147">147</a></td></tr
><tr id="gr_svn28_148"><td id="148"><a href="#148">148</a></td></tr
><tr id="gr_svn28_149"><td id="149"><a href="#149">149</a></td></tr
><tr id="gr_svn28_150"><td id="150"><a href="#150">150</a></td></tr
><tr id="gr_svn28_151"><td id="151"><a href="#151">151</a></td></tr
><tr id="gr_svn28_152"><td id="152"><a href="#152">152</a></td></tr
><tr id="gr_svn28_153"><td id="153"><a href="#153">153</a></td></tr
><tr id="gr_svn28_154"><td id="154"><a href="#154">154</a></td></tr
><tr id="gr_svn28_155"><td id="155"><a href="#155">155</a></td></tr
><tr id="gr_svn28_156"><td id="156"><a href="#156">156</a></td></tr
><tr id="gr_svn28_157"><td id="157"><a href="#157">157</a></td></tr
><tr id="gr_svn28_158"><td id="158"><a href="#158">158</a></td></tr
><tr id="gr_svn28_159"><td id="159"><a href="#159">159</a></td></tr
><tr id="gr_svn28_160"><td id="160"><a href="#160">160</a></td></tr
><tr id="gr_svn28_161"><td id="161"><a href="#161">161</a></td></tr
><tr id="gr_svn28_162"><td id="162"><a href="#162">162</a></td></tr
><tr id="gr_svn28_163"><td id="163"><a href="#163">163</a></td></tr
><tr id="gr_svn28_164"><td id="164"><a href="#164">164</a></td></tr
><tr id="gr_svn28_165"><td id="165"><a href="#165">165</a></td></tr
><tr id="gr_svn28_166"><td id="166"><a href="#166">166</a></td></tr
><tr id="gr_svn28_167"><td id="167"><a href="#167">167</a></td></tr
><tr id="gr_svn28_168"><td id="168"><a href="#168">168</a></td></tr
><tr id="gr_svn28_169"><td id="169"><a href="#169">169</a></td></tr
><tr id="gr_svn28_170"><td id="170"><a href="#170">170</a></td></tr
><tr id="gr_svn28_171"><td id="171"><a href="#171">171</a></td></tr
><tr id="gr_svn28_172"><td id="172"><a href="#172">172</a></td></tr
><tr id="gr_svn28_173"><td id="173"><a href="#173">173</a></td></tr
><tr id="gr_svn28_174"><td id="174"><a href="#174">174</a></td></tr
><tr id="gr_svn28_175"><td id="175"><a href="#175">175</a></td></tr
><tr id="gr_svn28_176"><td id="176"><a href="#176">176</a></td></tr
><tr id="gr_svn28_177"><td id="177"><a href="#177">177</a></td></tr
><tr id="gr_svn28_178"><td id="178"><a href="#178">178</a></td></tr
><tr id="gr_svn28_179"><td id="179"><a href="#179">179</a></td></tr
><tr id="gr_svn28_180"><td id="180"><a href="#180">180</a></td></tr
><tr id="gr_svn28_181"><td id="181"><a href="#181">181</a></td></tr
><tr id="gr_svn28_182"><td id="182"><a href="#182">182</a></td></tr
><tr id="gr_svn28_183"><td id="183"><a href="#183">183</a></td></tr
><tr id="gr_svn28_184"><td id="184"><a href="#184">184</a></td></tr
><tr id="gr_svn28_185"><td id="185"><a href="#185">185</a></td></tr
><tr id="gr_svn28_186"><td id="186"><a href="#186">186</a></td></tr
><tr id="gr_svn28_187"><td id="187"><a href="#187">187</a></td></tr
><tr id="gr_svn28_188"><td id="188"><a href="#188">188</a></td></tr
><tr id="gr_svn28_189"><td id="189"><a href="#189">189</a></td></tr
><tr id="gr_svn28_190"><td id="190"><a href="#190">190</a></td></tr
><tr id="gr_svn28_191"><td id="191"><a href="#191">191</a></td></tr
><tr id="gr_svn28_192"><td id="192"><a href="#192">192</a></td></tr
><tr id="gr_svn28_193"><td id="193"><a href="#193">193</a></td></tr
><tr id="gr_svn28_194"><td id="194"><a href="#194">194</a></td></tr
><tr id="gr_svn28_195"><td id="195"><a href="#195">195</a></td></tr
><tr id="gr_svn28_196"><td id="196"><a href="#196">196</a></td></tr
><tr id="gr_svn28_197"><td id="197"><a href="#197">197</a></td></tr
><tr id="gr_svn28_198"><td id="198"><a href="#198">198</a></td></tr
><tr id="gr_svn28_199"><td id="199"><a href="#199">199</a></td></tr
><tr id="gr_svn28_200"><td id="200"><a href="#200">200</a></td></tr
><tr id="gr_svn28_201"><td id="201"><a href="#201">201</a></td></tr
><tr id="gr_svn28_202"><td id="202"><a href="#202">202</a></td></tr
><tr id="gr_svn28_203"><td id="203"><a href="#203">203</a></td></tr
><tr id="gr_svn28_204"><td id="204"><a href="#204">204</a></td></tr
><tr id="gr_svn28_205"><td id="205"><a href="#205">205</a></td></tr
><tr id="gr_svn28_206"><td id="206"><a href="#206">206</a></td></tr
><tr id="gr_svn28_207"><td id="207"><a href="#207">207</a></td></tr
><tr id="gr_svn28_208"><td id="208"><a href="#208">208</a></td></tr
><tr id="gr_svn28_209"><td id="209"><a href="#209">209</a></td></tr
><tr id="gr_svn28_210"><td id="210"><a href="#210">210</a></td></tr
><tr id="gr_svn28_211"><td id="211"><a href="#211">211</a></td></tr
><tr id="gr_svn28_212"><td id="212"><a href="#212">212</a></td></tr
><tr id="gr_svn28_213"><td id="213"><a href="#213">213</a></td></tr
><tr id="gr_svn28_214"><td id="214"><a href="#214">214</a></td></tr
><tr id="gr_svn28_215"><td id="215"><a href="#215">215</a></td></tr
><tr id="gr_svn28_216"><td id="216"><a href="#216">216</a></td></tr
><tr id="gr_svn28_217"><td id="217"><a href="#217">217</a></td></tr
><tr id="gr_svn28_218"><td id="218"><a href="#218">218</a></td></tr
><tr id="gr_svn28_219"><td id="219"><a href="#219">219</a></td></tr
><tr id="gr_svn28_220"><td id="220"><a href="#220">220</a></td></tr
><tr id="gr_svn28_221"><td id="221"><a href="#221">221</a></td></tr
><tr id="gr_svn28_222"><td id="222"><a href="#222">222</a></td></tr
><tr id="gr_svn28_223"><td id="223"><a href="#223">223</a></td></tr
><tr id="gr_svn28_224"><td id="224"><a href="#224">224</a></td></tr
><tr id="gr_svn28_225"><td id="225"><a href="#225">225</a></td></tr
><tr id="gr_svn28_226"><td id="226"><a href="#226">226</a></td></tr
><tr id="gr_svn28_227"><td id="227"><a href="#227">227</a></td></tr
><tr id="gr_svn28_228"><td id="228"><a href="#228">228</a></td></tr
><tr id="gr_svn28_229"><td id="229"><a href="#229">229</a></td></tr
><tr id="gr_svn28_230"><td id="230"><a href="#230">230</a></td></tr
><tr id="gr_svn28_231"><td id="231"><a href="#231">231</a></td></tr
></table></pre>

<pre><table width="100%"><tr class="nocursor"><td></td></tr></table></pre>
</td>
<td id="lines">
<pre class="prettyprint"><table width="100%"><tr class="cursor_stop cursor_hidden"><td></td></tr></table></pre>

<pre class="prettyprint lang-lua"><table><tr
id=sl_svn28_1><td class="source">local xMax = math.floor( 128 / 6 ) - 1<br></td></tr
><tr
id=sl_svn28_2><td class="source">local yMax = math.floor( 96 / 8 ) - 1<br></td></tr
><tr
id=sl_svn28_3><td class="source">local game_map = {}<br></td></tr
><tr
id=sl_svn28_4><td class="source"><br></td></tr
><tr
id=sl_svn28_5><td class="source">local Head = {}<br></td></tr
><tr
id=sl_svn28_6><td class="source">local Tail = {}<br></td></tr
><tr
id=sl_svn28_7><td class="source"><br></td></tr
><tr
id=sl_svn28_8><td class="source">local highscore = 0<br></td></tr
><tr
id=sl_svn28_9><td class="source">local size = 3<br></td></tr
><tr
id=sl_svn28_10><td class="source">Tail.x = 1<br></td></tr
><tr
id=sl_svn28_11><td class="source">Tail.y = 1<br></td></tr
><tr
id=sl_svn28_12><td class="source">Head.x = Tail.x + ( size - 1 )<br></td></tr
><tr
id=sl_svn28_13><td class="source">Head.y = Tail.y<br></td></tr
><tr
id=sl_svn28_14><td class="source"><br></td></tr
><tr
id=sl_svn28_15><td class="source">local Food = {}<br></td></tr
><tr
id=sl_svn28_16><td class="source">Food.x = false<br></td></tr
><tr
id=sl_svn28_17><td class="source">Food.y = false<br></td></tr
><tr
id=sl_svn28_18><td class="source"><br></td></tr
><tr
id=sl_svn28_19><td class="source"><br></td></tr
><tr
id=sl_svn28_20><td class="source">Head.dx = 1<br></td></tr
><tr
id=sl_svn28_21><td class="source">Head.dy = 0<br></td></tr
><tr
id=sl_svn28_22><td class="source">Tail.dx = Head.dx<br></td></tr
><tr
id=sl_svn28_23><td class="source">Tail.dy = Head.dy<br></td></tr
><tr
id=sl_svn28_24><td class="source">local direction = &quot;right&quot;<br></td></tr
><tr
id=sl_svn28_25><td class="source">local level = 1<br></td></tr
><tr
id=sl_svn28_26><td class="source">local score = 0<br></td></tr
><tr
id=sl_svn28_27><td class="source"><br></td></tr
><tr
id=sl_svn28_28><td class="source">lm3s.disp.init( 1000000 )<br></td></tr
><tr
id=sl_svn28_29><td class="source"><br></td></tr
><tr
id=sl_svn28_30><td class="source"><br></td></tr
><tr
id=sl_svn28_31><td class="source">local kit = require( pd.board() )<br></td></tr
><tr
id=sl_svn28_32><td class="source"><br></td></tr
><tr
id=sl_svn28_33><td class="source">local pressed = {}<br></td></tr
><tr
id=sl_svn28_34><td class="source"><br></td></tr
><tr
id=sl_svn28_35><td class="source"><br></td></tr
><tr
id=sl_svn28_36><td class="source">local function create_food()<br></td></tr
><tr
id=sl_svn28_37><td class="source">--	if not food then<br></td></tr
><tr
id=sl_svn28_38><td class="source">	    print&quot;comida!!!&quot;<br></td></tr
><tr
id=sl_svn28_39><td class="source">			Food.x, Food.y = math.random( xMax - 1), math.random( yMax - 1)<br></td></tr
><tr
id=sl_svn28_40><td class="source">			while game_map[ Food.x ][ Food.y ] do<br></td></tr
><tr
id=sl_svn28_41><td class="source">			  Food.x, Food.y = math.random( xMax - 1 ), math.random( yMax - 1 )<br></td></tr
><tr
id=sl_svn28_42><td class="source">			end<br></td></tr
><tr
id=sl_svn28_43><td class="source">			game_map[ Food.x ][ Food.y ] = &quot;food&quot;<br></td></tr
><tr
id=sl_svn28_44><td class="source">			print(Food.x, Food.y)<br></td></tr
><tr
id=sl_svn28_45><td class="source">			lm3s.disp.print( &quot;@&quot;, Food.x * 6, Food.y * 8, 10 )<br></td></tr
><tr
id=sl_svn28_46><td class="source">--	end<br></td></tr
><tr
id=sl_svn28_47><td class="source">end<br></td></tr
><tr
id=sl_svn28_48><td class="source"><br></td></tr
><tr
id=sl_svn28_49><td class="source"><br></td></tr
><tr
id=sl_svn28_50><td class="source">local function eat_food()<br></td></tr
><tr
id=sl_svn28_51><td class="source">  print&quot;comi&quot;<br></td></tr
><tr
id=sl_svn28_52><td class="source">	lm3s.disp.print( &quot;@&quot;, Head.x * 6, Head.y * 8, 0 )<br></td></tr
><tr
id=sl_svn28_53><td class="source">	game_map[ Head.x ][ Head.y ] = nil<br></td></tr
><tr
id=sl_svn28_54><td class="source">	create_food()<br></td></tr
><tr
id=sl_svn28_55><td class="source">	score = score + level<br></td></tr
><tr
id=sl_svn28_56><td class="source">end<br></td></tr
><tr
id=sl_svn28_57><td class="source"><br></td></tr
><tr
id=sl_svn28_58><td class="source">local function check_collision()<br></td></tr
><tr
id=sl_svn28_59><td class="source">	if Head.x &lt;= 0 or Head.x &gt;= xMax then<br></td></tr
><tr
id=sl_svn28_60><td class="source">    return true<br></td></tr
><tr
id=sl_svn28_61><td class="source">  elseif Head.y &lt;= 0 or Head.y &gt;= yMax then<br></td></tr
><tr
id=sl_svn28_62><td class="source">    return true<br></td></tr
><tr
id=sl_svn28_63><td class="source">  elseif ( ( game_map[ Head.x ][ Head.y ] ) and ( game_map[ Head.x ][ Head.y ] ~= &quot;food&quot; ) ) then<br></td></tr
><tr
id=sl_svn28_64><td class="source">  	return true<br></td></tr
><tr
id=sl_svn28_65><td class="source">  end<br></td></tr
><tr
id=sl_svn28_66><td class="source">	  return false<br></td></tr
><tr
id=sl_svn28_67><td class="source">end<br></td></tr
><tr
id=sl_svn28_68><td class="source"><br></td></tr
><tr
id=sl_svn28_69><td class="source">local function move()<br></td></tr
><tr
id=sl_svn28_70><td class="source">	if game_map[ Tail.x ][ Tail.y ] == &quot;right&quot; then<br></td></tr
><tr
id=sl_svn28_71><td class="source">		Tail.dx = 1<br></td></tr
><tr
id=sl_svn28_72><td class="source">		Tail.dy = 0<br></td></tr
><tr
id=sl_svn28_73><td class="source">	elseif game_map[ Tail.x ][ Tail.y ] == &quot;left&quot; then<br></td></tr
><tr
id=sl_svn28_74><td class="source">		Tail.dx = -1<br></td></tr
><tr
id=sl_svn28_75><td class="source">		Tail.dy = 0<br></td></tr
><tr
id=sl_svn28_76><td class="source">	elseif game_map[ Tail.x ][ Tail.y ] == &quot;up&quot; then<br></td></tr
><tr
id=sl_svn28_77><td class="source">		Tail.dx = 0<br></td></tr
><tr
id=sl_svn28_78><td class="source">		Tail.dy = -1<br></td></tr
><tr
id=sl_svn28_79><td class="source">	elseif game_map[ Tail.x ][ Tail.y ] == &quot;down&quot; then<br></td></tr
><tr
id=sl_svn28_80><td class="source">		Tail.dx = 0<br></td></tr
><tr
id=sl_svn28_81><td class="source">		Tail.dy = 1<br></td></tr
><tr
id=sl_svn28_82><td class="source">	end<br></td></tr
><tr
id=sl_svn28_83><td class="source"><br></td></tr
><tr
id=sl_svn28_84><td class="source">	game_map[ Head.x ][ Head.y ] = direction<br></td></tr
><tr
id=sl_svn28_85><td class="source">	Head.x = Head.x + Head.dx<br></td></tr
><tr
id=sl_svn28_86><td class="source">	Head.y = Head.y + Head.dy<br></td></tr
><tr
id=sl_svn28_87><td class="source"><br></td></tr
><tr
id=sl_svn28_88><td class="source">	if game_map[ Head.x ][ Head.y ] == &quot;food&quot; then<br></td></tr
><tr
id=sl_svn28_89><td class="source">		eat_food()<br></td></tr
><tr
id=sl_svn28_90><td class="source">	else<br></td></tr
><tr
id=sl_svn28_91><td class="source">		lm3s.disp.print( &quot;*&quot;, Tail.x * 6, Tail.y * 8, 0 )<br></td></tr
><tr
id=sl_svn28_92><td class="source">		game_map[ Tail.x ][ Tail.y ] = nil<br></td></tr
><tr
id=sl_svn28_93><td class="source">		Tail.x = Tail.x + Tail.dx<br></td></tr
><tr
id=sl_svn28_94><td class="source">		Tail.y = Tail.y + Tail.dy<br></td></tr
><tr
id=sl_svn28_95><td class="source">	end<br></td></tr
><tr
id=sl_svn28_96><td class="source"><br></td></tr
><tr
id=sl_svn28_97><td class="source">	lm3s.disp.print( &quot;*&quot;, Head.x * 6, Head.y * 8, 10 )<br></td></tr
><tr
id=sl_svn28_98><td class="source"><br></td></tr
><tr
id=sl_svn28_99><td class="source">end<br></td></tr
><tr
id=sl_svn28_100><td class="source"><br></td></tr
><tr
id=sl_svn28_101><td class="source"><br></td></tr
><tr
id=sl_svn28_102><td class="source"><br></td></tr
><tr
id=sl_svn28_103><td class="source">local function draw_walls()<br></td></tr
><tr
id=sl_svn28_104><td class="source">print&quot;desenhando&quot;<br></td></tr
><tr
id=sl_svn28_105><td class="source">	for i = 0, xMax*2, 1 do<br></td></tr
><tr
id=sl_svn28_106><td class="source">    lm3s.disp.print( &quot;_&quot;, i * 3, yMax * 8 - 6, 11 )<br></td></tr
><tr
id=sl_svn28_107><td class="source">		lm3s.disp.print( &quot;_&quot;, i * 3, 0, 11 )<br></td></tr
><tr
id=sl_svn28_108><td class="source">	end<br></td></tr
><tr
id=sl_svn28_109><td class="source">  for i = 0, yMax*2, 1 do<br></td></tr
><tr
id=sl_svn28_110><td class="source">    lm3s.disp.print( &quot;|&quot;, xMax * 6, i * 4, 11 )<br></td></tr
><tr
id=sl_svn28_111><td class="source">		lm3s.disp.print( &quot;|&quot;, 0, i * 4, 11 )<br></td></tr
><tr
id=sl_svn28_112><td class="source">	end<br></td></tr
><tr
id=sl_svn28_113><td class="source">end<br></td></tr
><tr
id=sl_svn28_114><td class="source"><br></td></tr
><tr
id=sl_svn28_115><td class="source">local function button_clicked( button )<br></td></tr
><tr
id=sl_svn28_116><td class="source">  if kit.btn_pressed( button ) then<br></td></tr
><tr
id=sl_svn28_117><td class="source">    pressed[ button ] = true<br></td></tr
><tr
id=sl_svn28_118><td class="source">  else<br></td></tr
><tr
id=sl_svn28_119><td class="source">    if pressed[ button ] then<br></td></tr
><tr
id=sl_svn28_120><td class="source">      pressed[ button ] = nil<br></td></tr
><tr
id=sl_svn28_121><td class="source">      return true<br></td></tr
><tr
id=sl_svn28_122><td class="source">    end<br></td></tr
><tr
id=sl_svn28_123><td class="source">  end<br></td></tr
><tr
id=sl_svn28_124><td class="source">  return false<br></td></tr
><tr
id=sl_svn28_125><td class="source">end<br></td></tr
><tr
id=sl_svn28_126><td class="source"><br></td></tr
><tr
id=sl_svn28_127><td class="source"><br></td></tr
><tr
id=sl_svn28_128><td class="source">function init()<br></td></tr
><tr
id=sl_svn28_129><td class="source">  food = false<br></td></tr
><tr
id=sl_svn28_130><td class="source">  lm3s.disp.clear()<br></td></tr
><tr
id=sl_svn28_131><td class="source">	draw_walls()<br></td></tr
><tr
id=sl_svn28_132><td class="source">  size = 3<br></td></tr
><tr
id=sl_svn28_133><td class="source">	score = 0<br></td></tr
><tr
id=sl_svn28_134><td class="source">	level = 1<br></td></tr
><tr
id=sl_svn28_135><td class="source">  Tail.x = 1<br></td></tr
><tr
id=sl_svn28_136><td class="source">  Tail.y = 1<br></td></tr
><tr
id=sl_svn28_137><td class="source">  Head.x = Tail.x + ( size - 1 )<br></td></tr
><tr
id=sl_svn28_138><td class="source">  Head.y = Tail.y<br></td></tr
><tr
id=sl_svn28_139><td class="source">  Head.dx = 1<br></td></tr
><tr
id=sl_svn28_140><td class="source">  Head.dy = 0<br></td></tr
><tr
id=sl_svn28_141><td class="source">  Tail.dx = Head.dx<br></td></tr
><tr
id=sl_svn28_142><td class="source">  Tail.dy = Head.dy<br></td></tr
><tr
id=sl_svn28_143><td class="source">	direction = &quot;right&quot;<br></td></tr
><tr
id=sl_svn28_144><td class="source"><br></td></tr
><tr
id=sl_svn28_145><td class="source">	for i = 0, xMax, 1 do<br></td></tr
><tr
id=sl_svn28_146><td class="source">		game_map[ i ] = {}<br></td></tr
><tr
id=sl_svn28_147><td class="source">	end<br></td></tr
><tr
id=sl_svn28_148><td class="source">  for i = 0, size - 1, 1 do<br></td></tr
><tr
id=sl_svn28_149><td class="source">	  game_map[ Tail.x + ( i * Tail.dx ) ][ Tail.y + ( i * Tail.dy ) ] = direction<br></td></tr
><tr
id=sl_svn28_150><td class="source">	  lm3s.disp.print( &quot;*&quot;, ( Tail.x + ( i * Tail.dx ) ) * 6, ( Tail.y + ( i * Tail.dy ) ) * 8, 10 )<br></td></tr
><tr
id=sl_svn28_151><td class="source">  end<br></td></tr
><tr
id=sl_svn28_152><td class="source">	create_food()<br></td></tr
><tr
id=sl_svn28_153><td class="source">end<br></td></tr
><tr
id=sl_svn28_154><td class="source"><br></td></tr
><tr
id=sl_svn28_155><td class="source">--init()<br></td></tr
><tr
id=sl_svn28_156><td class="source">--create_food()<br></td></tr
><tr
id=sl_svn28_157><td class="source"><br></td></tr
><tr
id=sl_svn28_158><td class="source">repeat<br></td></tr
><tr
id=sl_svn28_159><td class="source">	init()<br></td></tr
><tr
id=sl_svn28_160><td class="source">  while true do<br></td></tr
><tr
id=sl_svn28_161><td class="source"><br></td></tr
><tr
id=sl_svn28_162><td class="source">		for i = 1, 1000 - ( 100 * level ), 1 do<br></td></tr
><tr
id=sl_svn28_163><td class="source">			if kit.btn_pressed( kit.BTN_RIGHT ) and direction ~= &quot;left&quot; then<br></td></tr
><tr
id=sl_svn28_164><td class="source">				direction = &quot;right&quot;<br></td></tr
><tr
id=sl_svn28_165><td class="source">				Head.dx = 1<br></td></tr
><tr
id=sl_svn28_166><td class="source">				Head.dy = 0<br></td></tr
><tr
id=sl_svn28_167><td class="source">			end<br></td></tr
><tr
id=sl_svn28_168><td class="source">			if kit.btn_pressed( kit.BTN_LEFT ) and direction ~= &quot;right&quot; then<br></td></tr
><tr
id=sl_svn28_169><td class="source">				direction = &quot;left&quot;<br></td></tr
><tr
id=sl_svn28_170><td class="source">				Head.dx = -1<br></td></tr
><tr
id=sl_svn28_171><td class="source">				Head.dy = 0<br></td></tr
><tr
id=sl_svn28_172><td class="source">			end<br></td></tr
><tr
id=sl_svn28_173><td class="source">			if kit.btn_pressed( kit.BTN_UP ) and direction ~= &quot;down&quot; then<br></td></tr
><tr
id=sl_svn28_174><td class="source">				direction = &quot;up&quot;<br></td></tr
><tr
id=sl_svn28_175><td class="source">				Head.dx = 0<br></td></tr
><tr
id=sl_svn28_176><td class="source">				Head.dy = -1<br></td></tr
><tr
id=sl_svn28_177><td class="source">			end<br></td></tr
><tr
id=sl_svn28_178><td class="source">			if kit.btn_pressed( kit.BTN_DOWN ) and direction ~= &quot;up&quot; then<br></td></tr
><tr
id=sl_svn28_179><td class="source">				direction = &quot;down&quot;<br></td></tr
><tr
id=sl_svn28_180><td class="source">				Head.dx = 0<br></td></tr
><tr
id=sl_svn28_181><td class="source">				Head.dy = 1<br></td></tr
><tr
id=sl_svn28_182><td class="source">			end<br></td></tr
><tr
id=sl_svn28_183><td class="source">			if button_clicked( kit.BTN_SELECT ) and level &lt; 10 then<br></td></tr
><tr
id=sl_svn28_184><td class="source">				level = level + 1<br></td></tr
><tr
id=sl_svn28_185><td class="source">			end<br></td></tr
><tr
id=sl_svn28_186><td class="source">		end<br></td></tr
><tr
id=sl_svn28_187><td class="source">		move()<br></td></tr
><tr
id=sl_svn28_188><td class="source">		if check_collision() then break end<br></td></tr
><tr
id=sl_svn28_189><td class="source">	  --tmr.delay( 0, 400000 )<br></td></tr
><tr
id=sl_svn28_190><td class="source">  	--[[<br></td></tr
><tr
id=sl_svn28_191><td class="source">	  game_map[ Head.x ][ Head.y ] = nil<br></td></tr
><tr
id=sl_svn28_192><td class="source">  	if Head.x &lt; xMax then<br></td></tr
><tr
id=sl_svn28_193><td class="source">	  	Head.x = Head.x + 1<br></td></tr
><tr
id=sl_svn28_194><td class="source">  	end<br></td></tr
><tr
id=sl_svn28_195><td class="source">  	game_map[ Head.x ][ Head.y ] = true<br></td></tr
><tr
id=sl_svn28_196><td class="source">    ]]--<br></td></tr
><tr
id=sl_svn28_197><td class="source">		collectgarbage( &quot;collect&quot; )<br></td></tr
><tr
id=sl_svn28_198><td class="source">	end<br></td></tr
><tr
id=sl_svn28_199><td class="source"><br></td></tr
><tr
id=sl_svn28_200><td class="source">	if score &gt; highscore then<br></td></tr
><tr
id=sl_svn28_201><td class="source">    highscore = score<br></td></tr
><tr
id=sl_svn28_202><td class="source">  end<br></td></tr
><tr
id=sl_svn28_203><td class="source">    lm3s.disp.clear()                         -- This statements displays the game over screen<br></td></tr
><tr
id=sl_svn28_204><td class="source">    lm3s.disp.print( &quot;Game Over :(&quot;, 30, 20, 11 )<br></td></tr
><tr
id=sl_svn28_205><td class="source">    lm3s.disp.print( &quot;Your score was &quot;..tostring( score ), 0, 40, 11 )<br></td></tr
><tr
id=sl_svn28_206><td class="source">    lm3s.disp.print( &quot;Highscore: &quot;..tostring( highscore ), 15, 50, 11 )<br></td></tr
><tr
id=sl_svn28_207><td class="source">    lm3s.disp.print( &quot;SELECT to restart&quot;, 6, 70, 11 )<br></td></tr
><tr
id=sl_svn28_208><td class="source">  enough = true                        -- If the player presses select before the time reach 1000000ms, then restart the game<br></td></tr
><tr
id=sl_svn28_209><td class="source">  for i=1, 1000000 do<br></td></tr
><tr
id=sl_svn28_210><td class="source">    if kit.btn_pressed( kit.BTN_SELECT ) then<br></td></tr
><tr
id=sl_svn28_211><td class="source">      enough = false<br></td></tr
><tr
id=sl_svn28_212><td class="source">      break<br></td></tr
><tr
id=sl_svn28_213><td class="source">    end<br></td></tr
><tr
id=sl_svn28_214><td class="source">  end<br></td></tr
><tr
id=sl_svn28_215><td class="source">  lm3s.disp.clear()<br></td></tr
><tr
id=sl_svn28_216><td class="source">until ( enough )<br></td></tr
><tr
id=sl_svn28_217><td class="source">lm3s.disp.off()<br></td></tr
><tr
id=sl_svn28_218><td class="source">--[[<br></td></tr
><tr
id=sl_svn28_219><td class="source">	for i in ipairs( game_map ) do<br></td></tr
><tr
id=sl_svn28_220><td class="source">		for j = 1, yMax, 1 do<br></td></tr
><tr
id=sl_svn28_221><td class="source">		  if game_map[ i ][ j ] then<br></td></tr
><tr
id=sl_svn28_222><td class="source">			  lm3s.disp.print( &quot;*&quot;, i * 6, j * 8, 10 )<br></td></tr
><tr
id=sl_svn28_223><td class="source">			else<br></td></tr
><tr
id=sl_svn28_224><td class="source">			  lm3s.disp.print( &quot;*&quot;, i * 6, j * 8, 0 )<br></td></tr
><tr
id=sl_svn28_225><td class="source">			end<br></td></tr
><tr
id=sl_svn28_226><td class="source">		end<br></td></tr
><tr
id=sl_svn28_227><td class="source">		--for j in ipairs ( game_map[ i ] ) do<br></td></tr
><tr
id=sl_svn28_228><td class="source">			--lm3s.disp.print( &quot;*&quot;, i * 6, j * 8, 10 )<br></td></tr
><tr
id=sl_svn28_229><td class="source">		--end<br></td></tr
><tr
id=sl_svn28_230><td class="source">	end<br></td></tr
><tr
id=sl_svn28_231><td class="source">	--]]<br></td></tr
></table></pre>

<pre class="prettyprint"><table width="100%"><tr class="cursor_stop cursor_hidden"><td></td></tr></table></pre>
</td>
</tr></table>



 <div id="log">
 <div style="text-align:right">
 <a class="ifCollapse" href="#" onclick="_toggleMeta('p', 'eluaexamples', this)">Show details</a>
 <a class="ifExpand" href="#" onclick="_toggleMeta('p', 'eluaexamples', this)">Hide details</a>
 </div>
 <div class="ifExpand">
 
 <div class="pmeta_bubble_bg" style="border:1px solid white">
 <div class="round4"></div>
 <div class="round2"></div>
 <div class="round1"></div>
 <div class="box-inner">
 <div id="changelog">
 <p>Change log</p>
 <div>
 <a href="/p/eluaexamples/source/detail?spec=svn28&r=28">r28</a>
 by teo.benjamin
 on Nov 17 (2 days ago)
 &nbsp; <a href="/p/eluaexamples/source/diff?spec=svn28&r=28&amp;format=side&amp;path=/trunk/snake/snake.lua&amp;old_path=/trunk/snake/snake.lua&amp;old=">Diff</a>
 </div>
 <pre>Adding the snake game.</pre>
 </div>
 
 
 
 
 
 
 <script type="text/javascript">
 var detail_url = '/p/eluaexamples/source/detail?r=28&spec=svn28';
 var publish_url = '/p/eluaexamples/source/detail?r=28&spec=svn28#publish';
 // describe the paths of this revision in javascript.
 var changed_paths = [];
 var changed_urls = [];
 
 changed_paths.push('/trunk/snake');
 changed_urls.push('/p/eluaexamples/source/browse/trunk/snake?r=28&spec=svn28');
 
 
 changed_paths.push('/trunk/snake/EK-LM3S8962.lua');
 changed_urls.push('/p/eluaexamples/source/browse/trunk/snake/EK-LM3S8962.lua?r=28&spec=svn28');
 
 
 changed_paths.push('/trunk/snake/snake.lua');
 changed_urls.push('/p/eluaexamples/source/browse/trunk/snake/snake.lua?r=28&spec=svn28');
 
 var selected_path = '/trunk/snake/snake.lua';
 
 
 function getCurrentPageIndex() {
 for (var i = 0; i < changed_paths.length; i++) {
 if (selected_path == changed_paths[i]) {
 return i;
 }
 }
 }
 function getNextPage() {
 var i = getCurrentPageIndex();
 if (i < changed_paths.length - 1) {
 return changed_urls[i + 1];
 }
 return null;
 }
 function getPreviousPage() {
 var i = getCurrentPageIndex();
 if (i > 0) {
 return changed_urls[i - 1];
 }
 return null;
 }
 function gotoNextPage() {
 var page = getNextPage();
 if (!page) {
 page = detail_url;
 }
 window.location = page;
 }
 function gotoPreviousPage() {
 var page = getPreviousPage();
 if (!page) {
 page = detail_url;
 }
 window.location = page;
 }
 function gotoDetailPage() {
 window.location = detail_url;
 }
 function gotoPublishPage() {
 window.location = publish_url;
 }
</script>
 
 <style type="text/css">
 #review_nav {
 border-top: 3px solid white;
 padding-top: 6px;
 margin-top: 1em;
 }
 #review_nav td {
 vertical-align: middle;
 }
 #review_nav select {
 margin: .5em 0;
 }
 </style>
 <div id="review_nav">
 <table><tr><td>Go to:&nbsp;</td><td>
 <select name="files_in_rev" onchange="window.location=this.value">
 
 <option value="/p/eluaexamples/source/browse/trunk/snake?r=28&amp;spec=svn28"
 
 >/trunk/snake</option>
 
 <option value="/p/eluaexamples/source/browse/trunk/snake/EK-LM3S8962.lua?r=28&amp;spec=svn28"
 
 >/trunk/snake/EK-LM3S8962.lua</option>
 
 <option value="/p/eluaexamples/source/browse/trunk/snake/snake.lua?r=28&amp;spec=svn28"
 selected="selected"
 >/trunk/snake/snake.lua</option>
 
 </select>
 </td></tr></table>
 
 
 




 <div style="white-space:nowrap">
 Project members,
 <a href="https://www.google.com/accounts/ServiceLogin?service=code&ltmpl=phosting&continue=http%3A%2F%2Fcode.google.com%2Fp%2Feluaexamples%2Fsource%2Fbrowse%2Ftrunk%2Fsnake%2Fsnake.lua&amp;followup=http%3A%2F%2Fcode.google.com%2Fp%2Feluaexamples%2Fsource%2Fbrowse%2Ftrunk%2Fsnake%2Fsnake.lua"
 >sign in</a> to write a code review</div>


 
 </div>
 
 
 </div>
 <div class="round1"></div>
 <div class="round2"></div>
 <div class="round4"></div>
 </div>
 <div class="pmeta_bubble_bg" style="border:1px solid white">
 <div class="round4"></div>
 <div class="round2"></div>
 <div class="round1"></div>
 <div class="box-inner">
 <div id="older_bubble">
 <p>Older revisions</p>
 
 <a href="/p/eluaexamples/source/list?path=/trunk/snake/snake.lua&start=28">All revisions of this file</a>
 </div>
 </div>
 <div class="round1"></div>
 <div class="round2"></div>
 <div class="round4"></div>
 </div>
 <div class="pmeta_bubble_bg" style="border:1px solid white">
 <div class="round4"></div>
 <div class="round2"></div>
 <div class="round1"></div>
 <div class="box-inner">
 <div id="fileinfo_bubble">
 <p>File info</p>
 
 <div>Size: 5087 bytes,
 231 lines</div>
 
 <div><a href="http://eluaexamples.googlecode.com/svn/trunk/snake/snake.lua">View raw file</a></div>
 </div>
 
 <div id="props">
 <p>File properties</p>
 <dl>
 
 <dt>svn:eol-style</dt>
 <dd>native</dd>
 
 </dl>
 </div>
 
 </div>
 <div class="round1"></div>
 <div class="round2"></div>
 <div class="round4"></div>
 </div>
 </div>
 </div>


</div>
</div>

 <script src="http://www.gstatic.com/codesite/ph/2429932258724909799/js/prettify/prettify.js"></script>

 <script src="http://www.gstatic.com/codesite/ph/2429932258724909799/js/prettify/lang-lua.js"></script>

<script type="text/javascript">prettyPrint();</script>

<script src="http://www.gstatic.com/codesite/ph/2429932258724909799/js/source_file_scripts_20080115.js"></script>

 <script type="text/javascript" src="http://kibbles.googlecode.com/files/kibbles-1.3.1.comp.js"></script>
 <script type="text/javascript">
 var lastStop = null;
 var initilized = false;
 
 function updateCursor(next, prev) {
 if (prev && prev.element) {
 prev.element.className = 'cursor_stop cursor_hidden';
 }
 if (next && next.element) {
 next.element.className = 'cursor_stop cursor';
 lastStop = next.index;
 }
 }
 
 function pubRevealed(data) {
 updateCursorForCell(data.cellId, 'cursor_stop cursor_hidden');
 if (initilized) {
 reloadCursors();
 }
 }
 
 function draftRevealed(data) {
 updateCursorForCell(data.cellId, 'cursor_stop cursor_hidden');
 if (initilized) {
 reloadCursors();
 }
 }
 
 function draftDestroyed(data) {
 updateCursorForCell(data.cellId, 'nocursor');
 if (initilized) {
 reloadCursors();
 }
 }
 function reloadCursors() {
 kibbles.skipper.reset();
 loadCursors();
 if (lastStop != null) {
 kibbles.skipper.setCurrentStop(lastStop);
 }
 }
 // possibly the simplest way to insert any newly added comments
 // is to update the class of the corresponding cursor row,
 // then refresh the entire list of rows.
 function updateCursorForCell(cellId, className) {
 var cell = document.getElementById(cellId);
 // we have to go two rows back to find the cursor location
 var row = getPreviousElement(cell.parentNode);
 row.className = className;
 }
 // returns the previous element, ignores text nodes.
 function getPreviousElement(e) {
 var element = e.previousSibling;
 if (element.nodeType == 3) {
 element = element.previousSibling;
 }
 if (element && element.tagName) {
 return element;
 }
 }
 function loadCursors() {
 // register our elements with skipper
 var elements = CR_getElements('*', 'cursor_stop');
 var len = elements.length;
 for (var i = 0; i < len; i++) {
 var element = elements[i]; 
 element.className = 'cursor_stop cursor_hidden';
 kibbles.skipper.append(element);
 }
 }
 function toggleComments() {
 CR_toggleCommentDisplay();
 reloadCursors();
 }
 function keysOnLoadHandler() {
 // setup skipper
 kibbles.skipper.addStopListener(
 kibbles.skipper.LISTENER_TYPE.PRE, updateCursor);
 // Set the 'offset' option to return the middle of the client area
 // an option can be a static value, or a callback
 kibbles.skipper.setOption('padding_top', 50);
 // Set the 'offset' option to return the middle of the client area
 // an option can be a static value, or a callback
 kibbles.skipper.setOption('padding_bottom', 100);
 // Register our keys
 kibbles.skipper.addFwdKey("n");
 kibbles.skipper.addRevKey("p");
 kibbles.keys.addKeyPressListener(
 'u', function() { window.location = detail_url; });
 kibbles.keys.addKeyPressListener(
 'r', function() { window.location = detail_url + '#publish'; });
 
 kibbles.keys.addKeyPressListener('j', gotoNextPage);
 kibbles.keys.addKeyPressListener('k', gotoPreviousPage);
 
 
 }
 window.onload = function() {keysOnLoadHandler();};
 </script>


<!-- code review support -->
<script src="http://www.gstatic.com/codesite/ph/2429932258724909799/js/code_review_scripts_20081023.js"></script>
<script type="text/javascript">
 
 // the comment form template
 var form = '<div class="draft"><div class="header"><span class="title">Draft comment:</span></div>' +
 '<div class="body"><form onsubmit="return false;"><textarea id="$ID">$BODY</textarea><br>$ACTIONS</form></div>' +
 '</div>';
 // the comment "plate" template used for both draft and published comment "plates".
 var draft_comment = '<div class="draft" ondblclick="$ONDBLCLICK">' +
 '<div class="header"><span class="title">Draft comment:</span><span class="actions">$ACTIONS</span></div>' +
 '<pre id="$ID" class="body">$BODY</pre>' +
 '</div>';
 var published_comment = '<div class="published">' +
 '<div class="header"><span class="title"><a href="$PROFILE_URL">$AUTHOR:</a></span><div>' +
 '<pre id="$ID" class="body">$BODY</pre>' +
 '</div>';

 function showPublishInstructions() {
 var element = document.getElementById('review_instr');
 if (element) {
 element.className = 'opened';
 }
 }
 function revsOnLoadHandler() {
 // register our source container with the commenting code
 var paths = {'svn28': '/trunk/snake/snake.lua'}
 CR_setup('p', 'eluaexamples', '', 'svn28', paths,
 '', CR_BrowseIntegrationFactory);
 // register our hidden ui elements with the code commenting code ui builder.
 CR_registerLayoutElement('form', form);
 CR_registerLayoutElement('draft_comment', draft_comment);
 CR_registerLayoutElement('published_comment', published_comment);
 
 CR_registerActivityListener(CR_ACTIVITY_TYPE.REVEAL_DRAFT_PLATE, showPublishInstructions);
 
 CR_registerActivityListener(CR_ACTIVITY_TYPE.REVEAL_PUB_PLATE, pubRevealed);
 CR_registerActivityListener(CR_ACTIVITY_TYPE.REVEAL_DRAFT_PLATE, draftRevealed);
 CR_registerActivityListener(CR_ACTIVITY_TYPE.DISCARD_DRAFT_COMMENT, draftDestroyed);
 
 
 
 
 
 
 
 
 
 var initilized = true;
 reloadCursors();
 }
 window.onload = function() {keysOnLoadHandler(); revsOnLoadHandler();};
</script>

<script type="text/javascript" src="http://www.gstatic.com/codesite/ph/2429932258724909799/js/dit_scripts_20081013.js"></script>

 
 <script type="text/javascript" src="http://www.gstatic.com/codesite/ph/2429932258724909799/js/core_scripts_20081103.js"></script>
 <script type="text/javascript" src="/js/codesite_product_dictionary_ph.pack.04102009.js"></script>
 
 
 
 </div>
<div id="footer" dir="ltr">
 
 <div class="text">
 
 &copy;2009 Google -
 <a href="/">Code Home</a> -
 <a href="/projecthosting/terms.html">Terms of Service</a> -
 <a href="http://www.google.com/privacy.html">Privacy Policy</a> -
 <a href="/more/">Site Directory</a> -
 <a href="/p/support/">Project Hosting Help</a>
 
 </div>
</div>
<script type="text/javascript">
/**
 * Reports analytics.
 * It checks for the analytics functionality (window._gat) every 100ms
 * until the analytics script is fully loaded in order to invoke siteTracker.
 */
function _CS_reportAnalytics() {
 window.setTimeout(function() {
 if (window._gat) {
 try {
 siteTracker = _gat._getTracker(CS_ANALYTICS_ACCOUNT);
 siteTracker._trackPageview();
 } catch (e) {}
 
 } else {
 _CS_reportAnalytics();
 }
 }, 100);
}
</script>

 
 
 <div class="hostedBy" style="margin-top: -20px;">
 <span style="vertical-align: top;">Hosted by</span>
 <a href="/hosting/">
 <img src="http://www.gstatic.com/codesite/ph/images/google_code_tiny.png" width="107" height="24" alt="Google Code">
 </a>
 </div>
 
 
 
 


 
 </body>
</html>

