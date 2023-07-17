#!/usr/bin/env lua

-- generate the foreach.h file
-- would be nice if I could do this all in macros somehow
-- maybe with multiple variable redefinitions and multiple re-includes of the same file?

local n = ... and tonumber((...)) or 20

require 'ext.path'['foreach.h'] = require 'template'([[
#pragma once

<?
	for i=1,n do
?>#define FOR_EACH_<?=i?>(what, between, extra, x, ...) what(x, extra)<?
		if i > 1 then
?> between FOR_EACH_<?=i-1?>(what, DEFER(between), DEFER(extra), __VA_ARGS__)<?
		end ?>
<?	end
?>#define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
#define FOR_EACH_NARG_(...) FOR_EACH_ARG_N(__VA_ARGS__)
#define FOR_EACH_ARG_N(<?
	for i=1,n do
?>_<?=i?>, <?
	end
?>N, ...) N
#define FOR_EACH_RSEQ_N() <?
	for i=n,1,-1 do
?><?=i?>, <?
	end
?>0
#define FOR_EACH_(N, what, between, extra, ...) CONCAT(FOR_EACH_, N)(what, DEFER(between), DEFER(extra), __VA_ARGS__)
#define FOR_EACH(what, between, extra, ...) FOR_EACH_(FOR_EACH_NARG(__VA_ARGS__), what, DEFER(between), DEFER(extra), __VA_ARGS__)]]
, {n=n})
