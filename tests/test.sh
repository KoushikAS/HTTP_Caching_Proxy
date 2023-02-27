#!/bin/sh
echo "Starting Testing"


cat resource/bad-format-req.txt | netcat 127.0.0.1 12345  > actual/bad-format-res.txt

#testing the header code
diff --brief <(head -n 1 resource/bad-format-res.txt) <(head -n 1 actual/bad-format-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "bad-format request did not match with the stored value"
else
    echo "bad-format request Passed"
fi


cat resource/http-connect-req.txt | netcat 127.0.0.1 12345  > actual/http-connect-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-connect-res.txt) <(head -n 1 actual/http-connect-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http Connect did not match with the stored value"
else
    echo "Http Connect Request Passed"
fi

cat resource/http-get-404-notfound-req.txt | netcat 127.0.0.1 12345  > actual/http-get-404-notfound-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-404-notfound-res.txt) <(head -n 1 actual/http-get-404-notfound-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http Get 404-notfound did not match with the stored value"
else
    echo "Http Get 404-notfound Passed"
fi


cat resource/http-get-bad-request-req.txt | netcat 127.0.0.1 12345  > actual/http-get-bad-request-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-bad-request-res.txt) <(head -n 1 actual/http-get-bad-request-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http Get bad-request did not match with the stored value"
else
    echo "Http Get bad-request Passed"
fi


cat resource/http-get-internal-server-error-req.txt | netcat 127.0.0.1 12345  > actual/http-get-internal-server-error-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-internal-server-error-res.txt) <(head -n 1 actual/http-get-internal-server-error-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http Get internal-server-error did not match with the stored value"
else
    echo "Http Get internal-server-error Passed"
fi

cat resource/http-post-req.txt | netcat 127.0.0.1 12345  > actual/http-post-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-post-res.txt) <(head -n 1 actual/http-post-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http Post did not match with the stored value"
else
    echo "Http Post Passed"
fi


cat resource/http-get-chunked-req.txt | netcat 127.0.0.1 12345  > actual/http-get-chunked-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-chunked-res.txt) <(head -n 1 actual/http-get-chunked-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http chunked Get Request did not match witht stored value"
else
    echo "Http chunked Get Request Passed"
fi

cat resource/http-get-nocache-info-req.txt | netcat 127.0.0.1 12345  > actual/http-get-nocache-info-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-nocache-info-res.txt) <(head -n 1 actual/http-get-nocache-info-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http Get no cache info did not match with the stored value"
else
    echo "Http Get no cache info Request Passed"
fi

cat resource/http-get-no-cache-req.txt | netcat 127.0.0.1 12345  > actual/http-get-no-cache-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-no-cache-res.txt) <(head -n 1 actual/http-get-no-cache-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http Get no-cache did not match with the stored value"
else
    echo "Http Get no-cache Request Passed"
fi

cat resource/http-get-no-store-req.txt | netcat 127.0.0.1 12345  > actual/http-get-no-store-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-no-store-res.txt) <(head -n 1 actual/http-get-no-store-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http Get no-store did not match with the stored value"
else
    echo "Http Get no-store Request Passed"
fi

cat resource/http-get-private-req.txt | netcat 127.0.0.1 12345  > actual/http-get-private-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-private-res.txt) <(head -n 1 actual/http-get-private-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http Get private did not match with the stored value"
else
    echo "Http Get private Request Passed"
fi

cat resource/http-get-must-revalidate-withoutage-req.txt | netcat 127.0.0.1 12345  > actual/http-get-must-revalidate-withoutage-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-must-revalidate-withoutage-res.txt) <(head -n 1 actual/http-get-must-revalidate-withoutage-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http Get must-revalidate-withoutage did not match with the stored value"
else
    echo "Http Get must-revalidate-withoutage Request Passed"
fi


cat resource/http-get-maxage-req.txt | netcat 127.0.0.1 12345  > actual/http-get-maxage-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-maxage-res.txt) <(head -n 1 actual/http-get-maxage-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http Get maxage did not match with the stored value"
else
    echo "Http Get maxage Request Passed"
fi


cat resource/http-get-maxage-req.txt | netcat 127.0.0.1 12345  > actual/http-get-maxage-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-maxage-res.txt) <(head -n 1 actual/http-get-maxage-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http Get Cached maxage did not match with the stored value"
else
    echo "Http Get Cached maxage Request Passed"
fi

sleep 5

cat resource/http-get-maxage-req.txt | netcat 127.0.0.1 12345  > actual/http-get-maxage-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-maxage-res.txt) <(head -n 1 actual/http-get-maxage-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http Refetch from original server get maxage request did not match with the stored value"
else
    echo "Http  Refetch from original server get maxage request  Passed"
fi



cat resource/http-get-maxage-must-revalidate-req.txt | netcat 127.0.0.1 12345  > actual/http-get-maxage-must-revalidate-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-maxage-must-revalidate-res.txt) <(head -n 1 actual/http-get-maxage-must-revalidate-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http Get maxage-must-revalidate did not match with the stored value"
else
    echo "Http Get maxage-must-revalidate Request Passed"
fi


cat resource/http-get-maxage-must-revalidate-req.txt | netcat 127.0.0.1 12345  > actual/http-get-maxage-must-revalidate-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-maxage-must-revalidate-res.txt) <(head -n 1 actual/http-get-maxage-must-revalidate-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http Get Cached maxage-must-revalidate did not match with the stored value"
else
    echo "Http Get Cached maxage-must-revalidate Request Passed"
fi

sleep 5

cat resource/http-get-maxage-must-revalidate-req.txt | netcat 127.0.0.1 12345  > actual/http-get-maxage-must-revalidate-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-maxage-must-revalidate-res.txt) <(head -n 1 actual/http-get-maxage-must-revalidate-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http Refetch from original server max-must-revalidate request did not match with the stored value"
else
    echo "Http  Refetch from original server get maxage-must-revalidate request  Passed"
fi



echo "Finished Testing"
