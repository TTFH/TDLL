data = open("sentry.dll","rb").read()
old = b"api-ms-win-core-synch-l1-2-0.dll"
new = b"waitaddr_shim.dll".ljust(len(old), b"\0")
print("occurrences=", data.count(old)) # expect 1
open("sentry_patched.dll","wb").write(data.replace(old, new))
