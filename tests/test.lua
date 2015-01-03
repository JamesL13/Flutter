
function fmtTable(tab, indent)
    local result = ""
    for k, v in pairs(tab) do
        for j = 1, indent do
            result = result .. "&nbsp;"
        end
        result = result .. tostring(k) .. " : "
        if type(v) == "table" then
            result = result .. "<br />" .. fmtTable(v, indent+2)
        else
            result = result .. tostring(v) .. "<br />"
        end
    end
    return result
end

function onRequest(req)
    local content = fmtTable(req, 0)

    return {
        content="<html><body><p>"..content.."</p></body></html>",
        code=200,
        head={mimetype = "text/html"}}
end

