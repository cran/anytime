// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
// anytime: Use Boost Date_Time to convert date(time) data to POSIXt
//
// Copyright (C) 2015 - 2016  Dirk Eddelbuettel
//
// This file is part of anytime.
//
// anytime is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// anytime is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with anytime.  If not, see <http://www.gnu.org/licenses/>.

#include <boost/date_time.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/lexical_cast.hpp>
#include <Rcpp.h>

namespace bt = boost::posix_time;

const std::locale formats[] = {
    std::locale(std::locale::classic(), new bt::time_input_facet("%Y-%m-%d %H:%M:%S%f")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%Y/%m/%d %H:%M:%S%f")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%Y%m%d %H%M%S%f")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%Y%m%d %H:%M:%S%f")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%m/%d/%Y %H:%M:%S%f")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%m-%d-%Y %H:%M:%S%f")),
    // std::locale(std::locale::classic(), new bt::time_input_facet("%d.%m.%Y %H:%M:%S%f")),

    std::locale(std::locale::classic(), new bt::time_input_facet("%Y-%b-%d %H:%M:%S%f")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%Y/%b/%d %H:%M:%S%f")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%Y%b%d %H%M%S%F")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%Y%b%d %H:%M:%S%F")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%b/%d/%Y %H:%M:%S%f")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%b-%d-%Y %H:%M:%S%f")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%d.%b.%Y %H:%M:%S%f")),

    std::locale(std::locale::classic(), new bt::time_input_facet("%Y-%B-%d %H:%M:%S%f")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%Y/%B/%d %H:%M:%S%f")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%Y%B%d %H%M%S%f")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%Y%B%d %H:%M:%S%f")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%B/%d/%Y %H:%M:%S%f")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%B-%d-%Y %H:%M:%S%f")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%d.%B.%Y %H:%M:%S%f")),

    // see http://stackoverflow.com/questions/39259184/formatting-dates-with-r for next one
    std::locale(std::locale::classic(), new bt::time_input_facet("%a %b %d %H:%M:%S%F %Y")),

    std::locale(std::locale::classic(), new bt::time_input_facet("%Y-%m-%d")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%Y%m%d")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%m/%d/%Y")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%m-%d-%Y")),

    std::locale(std::locale::classic(), new bt::time_input_facet("%Y-%b-%d")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%Y%b%d")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%b/%d/%Y")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%b-%d-%Y")),

    std::locale(std::locale::classic(), new bt::time_input_facet("%Y-%B-%d")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%Y%B%d")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%B/%d/%Y")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%B-%d-%Y"))

};
const size_t nformats = sizeof(formats)/sizeof(formats[0]);

double stringToTime(const std::string s) {

    //This local adjustor depends on the machine TZ settings-- highly dangerous!
    typedef boost::date_time::c_local_adjustor<bt::ptime> local_adj;

    bt::ptime pt, ptbase;

    // loop over formats and try them til one fits
    for (size_t i=0; pt == ptbase && i < nformats; ++i) {
        std::istringstream is(s);
        is.imbue(formats[i]);
        is >> pt;
    }

    if (pt == ptbase) {
        return NAN;
    } else {
        const bt::ptime timet_start(boost::gregorian::date(1970,1,1));
        const bt::ptime local_timet_start = local_adj::utc_to_local(timet_start);

        // seconds sincr epoch (in local time) -- misses DST adjustment
        bt::time_duration tdiff = pt - local_timet_start;
        //Rcpp::Rcout << "tdiff is " << tdiff << std::endl;
        //Rcpp::Rcout << "pt is " << pt << std::endl;

        // hack-ish: go back to struct tm to use its tm_isdst field
        time_t secsSinceEpoch = tdiff.total_seconds();
        struct tm* localAsTm = localtime(&secsSinceEpoch);
        //Rcpp::Rcout << "Adj is " << localAsTm->tm_isdst << std::endl;

        // Define BOOST_DATE_TIME_POSIX_TIME_STD_CONFIG to use nanoseconds
        // (and then use diff.total_nanoseconds()/1.0e9;  instead)
        //
        // note dst correction here -- needed as UTC offset is correct but does not
        // contain the additional DST adjustment
        return tdiff.total_microseconds()/1.0e6 - localAsTm->tm_isdst*60*60;
    }
}

template <int RTYPE>
Rcpp::NumericVector anytime_impl(const Rcpp::Vector<RTYPE>& sv,
                                 const std::string& tz = "UTC") {

    int n = sv.size();
    Rcpp::NumericVector pv(n);
    pv.attr("class") = Rcpp::CharacterVector::create("POSIXct", "POSIXt");
    pv.attr("tzone") = tz;

    for (int i=0; i<n; i++) {
        std::string s = boost::lexical_cast<std::string>(sv[i]);
        //Rcpp::Rcout << sv[i] << " -- " << s << std::endl;

        // Boost Date_Time gets the 'YYYYMMDD' format wrong, even
        // when given as an explicit argument. So we need to test here.
        // While we're at it, may as well test for obviously wrong data.
        int l = s.size();
        if (l < 8) { 	        // too short
            Rcpp::stop("Inadmissable input: %s", s);
        } else if (l == 8) {    // turn YYYYMMDD into YYYY/MM/DD
            s = s.substr(0, 4) + "/" + s.substr(4, 2) + "/" + s.substr(6,2);
        }
        pv[i] = stringToTime(s);
    }
    return pv;
}


// [[Rcpp::export]]
Rcpp::NumericVector anytime_cpp(SEXP x, std::string tz = "UTC") {
    if (Rcpp::is<Rcpp::CharacterVector>(x)) {
        return anytime_impl<STRSXP>(x, tz);
    } else if (Rcpp::is<Rcpp::IntegerVector>(x)) {
        return anytime_impl<INTSXP>(x, tz);
    } else if (Rcpp::is<Rcpp::NumericVector>(x)) {
        // here we have two cases: either we are an int like
        // 200150315 'mistakenly' cast to numeric, or we actually
        // are a proper large numeric (ie as.numeric(Sys.time())
        Rcpp::NumericVector v(x);
        if (v[0] < 21990101) {  // somewhat arbitrary cuttoff
            // actual integer date notation: convert to string and parse
            return anytime_impl<REALSXP>(x, tz);
        } else {
            // we think it is a numeric time, so treat it as one
            return x;
        }
    } else {
        Rcpp::stop("Unsupported Type");
        return R_NilValue;//not reached
    }
}

