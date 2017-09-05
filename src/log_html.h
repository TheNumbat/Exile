
#pragma once

const string log_html_header = np_string_literal(R"STR(
<doctype html>
<html>
<head>
	<meta charset="utf-8">
	<title>Log</title>
	<style>
html {
	background: #eee;
}

table {
	width: 80%;
	margin: auto;	
	border-collapse: collapse;
	table-layout: fixed;
}

.header {
	border-bottom: 1px solid black;
}

td {
	white-space : nowrap;
	overflow: hidden;
	text-overflow : ellipsis;
	padding-right: 10px;
	padding-left: 10px;
	border-right: 1px solid black;
}
td:hover {
	white-space: initial;
	overflow-wrap: break-word;
	background: #aaa;
	transition: all 0.1s;
}

tr:hover {
	
}

.time {
	width: 6%;
}

.cstack {
	width: 16%;
}

.location {
	width: 9%;
}

.level {
	width: 5%;
	justify-content: left;
}

.message {
	width: 45%;
	border-right: 0px;
	padding-right: 0px;
}
</style>
</head>
<body>
<table>
<tr class="header"><th class="time">TIME</th><th class="cstack">CONTEXT</th><th class="location">LOCATION</th><th class="leveL">LEVEL</th><th class="message">MESSAGE</th></tr>
)STR");

const string log_html_footer = np_string_literal(R"STR(
</table>
</body>
</html>
)STR");

const string log_html_msg = np_string_literal(R"STR(<tr><td class="time">%</td><td class = "cstack">%</td><td class = "location">%</td><td class = "level">%</td><td class = "message">%</td></tr>)STR""\n");
