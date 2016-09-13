### anytime ###

Anything to 'POSIXct' Converter

### Motivation

R excels at computing with dates, and times.  Using _typed_ representation for your data is highly
recommended not only because of the functionality offered but also because of the added safety
stemming from proper representation.

But there is a small nuisance cost in interactive work as well as in programming. Users must have
told `as.POSIXct()` about a million times that the origin is (of course) the
[epoch](https://en.wikipedia.org/wiki/Unix_time). Do we really have to say it a million more times?
Similarly, when parsing dates that are _some form_ of YYYYMMDD format, do we really have to manually
convert from `integer` or `numeric` or `factor` or `ordered` to character? Having one of several
common separators and/or date / time month forms (YYYY-MM-DD, YYYY/MM/DD, YYYYMMDD, YYYY-mon-DD and
so on, with or without times), do we really need a format string? Or could a smart converter
function do this?

`anytime()` aims to be that _general purpose_ converter returning a proper `POSIXct` (or `Date`)
object nomatter the input (provided it was somewhat parseable), relying on
[Boost date_time](http://www.boost.org/doc/libs/1_61_0/doc/html/date_time.html) for the (efficient,
performant) conversion.

### Examples

#### From Integer or Numeric or Factor or Ordered

```r
R> library(anytime)
R> options(digits.secs=6)                ## for fractional seconds below
R> Sys.setenv(TZ=anytime:::getTZ())      ## helper function to try to get TZ
R>
R> ## integer    
R> anytime(20160101L + 0:2)
[1] "2016-01-01 CST" "2016-01-02 CST" "2016-01-03 CST"
R> 
R> ## numeric
R> anytime(20160101 + 0:2)
[1] "2016-01-01 CST" "2016-01-02 CST" "2016-01-03 CST"
R>
R> ## factor
R> anytime(as.factor(20160101 + 0:2))
[1] "2016-01-01 CST" "2016-01-02 CST" "2016-01-03 CST"
R>
R> ## ordered
R> anytime(as.ordered(20160101 + 0:2))
[1] "2016-01-01 CST" "2016-01-02 CST" "2016-01-03 CST"
```

#### Character: Simple

```r
R> ## Dates: Character
R> anytime(as.character(20160101 + 0:2))
[1] "2016-01-01 CST" "2016-01-02 CST" "2016-01-03 CST"
R>
R> ## Dates: alternate formats
R> anytime(c("20160101", "2016/01/02", "2016-01-03"))
[1] "2016-01-01 CST" "2016-01-02 CST" "2016-01-03 CST"
R>
```

#### Character: ISO

```r
R> ## Datetime: ISO with/without fractional seconds
R> anytime(c("2016-01-01 10:11:12", "2016-01-01 10:11:12.345678"))
[1] "2016-01-01 10:11:12.000000 CST" "2016-01-01 10:11:12.345678 CST"
R>
R> ## Datetime: ISO alternate (?) with 'T' separator  
R> anytime(c("20160101T101112", "20160101T101112.345678"))
[1] "2016-01-01 10:11:12.000000 CST" "2016-01-01 10:11:12.345678 CST"
```

#### Character: Textual month formats

```r
R> ## ISO style 
R> anytime(c("2016-Sep-01 10:11:12", "Sep/01/2016 10:11:12", "Sep-01-2016 10:11:12"))
[1] "2016-09-01 10:11:12 CDT" "2016-09-01 10:11:12 CDT" "2016-09-01 10:11:12 CDT"
R>
R> ## Datetime: Mixed format (cf http://stackoverflow.com/questions/39259184)
R> anytime(c("Thu Sep 01 10:11:12 2016", "Thu Sep 01 10:11:12.345678 2016"))
[1] "2016-09-01 10:11:12.000000 CDT" "2016-09-01 10:11:12.345678 CDT"
```


#### Character: Dealing with DST

This shows an important aspect. When not working localtime (by overriding to `UTC`) the _changing
difference_ UTC is correctly covered (which the underlying
[Boost Date_Time](http://www.boost.org/doc/libs/1_61_0/doc/html/date_time.html) library does not by
itself).


```r
R> ## Datetime: pre/post DST
R> anytime(c("2016-01-31 12:13:14", "2016-08-31 12:13:14"))
[1] "2016-01-31 12:13:14 CST" "2016-08-31 12:13:14 CDT"
R> anytime(c("2016-01-31 12:13:14", "2016-08-31 12:13:14"), tz="UTC")  # important: catches change
[1] "2016-01-31 18:13:14 UTC" "2016-08-31 17:13:14 UTC"
```

### Technical Details

The heavy lifting is done by a combination of
[Boost lexical_cast](http://www.boost.org/doc/libs/1_61_0/doc/html/boost_lexical_cast.html) to go
from _anything_ to string representation which is then parsed by
[Boost Date_Time](http://www.boost.org/doc/libs/1_61_0/doc/html/date_time.html).  We use the
[BH package](http://dirk.eddelbuettel.com/code/bh.html) to access [Boost](http://www.boost.org), and
rely on [Rcpp](http://dirk.eddelbuettel.com/code/rcpp.html) for a seamless C++ interface to and from
[R](https://www.r-project.org).

### Status

Works as expected. Not yet uploaded to CRAN.

### Author

Dirk Eddelbuettel

### License

GPL (>= 2)
