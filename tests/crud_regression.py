#!/usr/bin/env python3
"""CRUD Regression Test Script

This script verifies that all CRUD operations work correctly
for each entity type in the application.
"""

import json
import sys
import urllib.request
import urllib.error

BASE_URL = "http://127.0.0.1:8080/api"

def api_call(endpoint, method="GET", data=None):
    url = f"{BASE_URL}{endpoint}"
    headers = {"Content-Type": "application/json"}
    body = json.dumps(data).encode("utf-8") if data else None
    req = urllib.request.Request(url, data=body, headers=headers, method=method)
    try:
        with urllib.request.urlopen(req, timeout=10) as resp:
            return json.loads(resp.read().decode("utf-8")), resp.status
    except urllib.error.HTTPError as e:
        return json.loads(e.read().decode("utf-8")), e.code
    except urllib.error.URLError:
        return {"error": "Connection refused"}, 503

def test_crud(entity_name, endpoint, create_payload, update_field, update_value):
    results = []
    
    list_data, status = api_call(endpoint)
    results.append(("LIST", status == 200, f"Status: {status}"))
    
    create_data, status = api_call(endpoint, "POST", create_payload)
    created = status in (200, 201)
    results.append(("CREATE", created, f"Status: {status}"))
    
    entity_id = None
    if created and "id" in create_data:
        entity_id = create_data["id"]
    elif created:
        list_data2, _ = api_call(endpoint)
        if isinstance(list_data2, list) and len(list_data2) > 0:
            entity_id = list_data2[-1].get("id")
    
    if entity_id:
        get_data, status = api_call(f"{endpoint}/{entity_id}")
        results.append(("GET", status == 200, f"Status: {status}"))
        
        update_payload = {**create_payload, update_field: update_value}
        _, status = api_call(f"{endpoint}/{entity_id}", "PUT", update_payload)
        results.append(("UPDATE", status in (200, 204), f"Status: {status}"))
        
        _, status = api_call(f"{endpoint}/{entity_id}", "DELETE")
        results.append(("DELETE", status in (200, 204), f"Status: {status}"))
    else:
        results.append(("GET", False, "No ID available"))
        results.append(("UPDATE", False, "No ID available"))
        results.append(("DELETE", False, "No ID available"))
    
    return results

def main():
    if not sys.argv[1:]:
        print("Usage: python crud_regression.py [--skip-server-check]")
        print("Make sure the backend server is running on http://127.0.0.1:8080")
        sys.exit(1)

    entities = [
        ("Course", "/courses", {"name": "回归测试课程", "code": "REG101", "credits": 3.0, "semester": "2026春"}, "name", "更新后课程名"),
        ("Goal", "/goals", {"title": "回归测试目标", "description": "测试", "progress": 50}, "title", "更新后目标名"),
        ("Achievement", "/achievements", {"title": "回归测试成果", "level": "校级", "date": "2026-04-29"}, "title", "更新后成果名"),
        ("Experience", "/experiences", {"title": "回归测试经历", "organization": "测试组织"}, "title", "更新后经历名"),
        ("Role", "/roles", {"title": "回归测试角色", "organization": "测试组织"}, "title", "更新后角色名"),
        ("Activity", "/activities", {"name": "回归测试活动", "category": "学术"}, "name", "更新后活动名"),
    ]

    total_pass = 0
    total_fail = 0

    for entity_name, endpoint, create_payload, update_field, update_value in entities:
        print(f"\n=== {entity_name} ===")
        results = test_crud(entity_name, endpoint, create_payload, update_field, update_value)
        for op, passed, detail in results:
            status = "PASS" if passed else "FAIL"
            if passed:
                total_pass += 1
            else:
                total_fail += 1
            print(f"  {op}: {status} ({detail})")

    print(f"\n=== Summary ===")
    print(f"Total: {total_pass + total_fail}, Passed: {total_pass}, Failed: {total_fail}")
    
    if total_fail > 0:
        sys.exit(1)

if __name__ == "__main__":
    main()
