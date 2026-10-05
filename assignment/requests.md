# Health
Purpose: Confirm that Synapse is running correctly.
Type: `GET`
Url: `http://localhost:8008/health`
Expected result: `200 OK`
Expected response body:
```json
OK
```
# Login
Purpose: Obtain an access token required for authenticating user actions. 
Type: `POST`
Url: `http://localhost:8008/_matrix/client/v3/login`
Body:
```json
{
  "identifier": {
    "type": "m.id.user",
    "user": "<username>"
  },
  "password": "<password>",
  "type": "m.login.password"
}
```

Expected result: `200 OK`
Expected response body:
```json
{
  "user_id": "@<username>:localhost",
  "access_token": "syt_ZGlyaw_bnLxjdEhstImHfQCYdUv_0WZnUK",
  "home_server": "localhost",
  "device_id": "FXZQEBBFMQ"
}
```

> Save `access_token` for later

# Create room
Purpose: Make a new chatroom to create a clean environment in which to test message sending.
Type: `POST`
Url: `http://localhost:8008/_matrix/client/v3/createRoom`
Body:
```json
{
  "name": "My test room"
}
```

Headers:
```json
authorization: Bearer <access_token>
```

Expected result: `200 OK`
Expected response body:
```json
{
  "room_id": "!WkeejZBBZhtBrNiwsj:localhost"
}
```

> Save `room_id` for later

# Send message
Purpose: This is the primary action we want to test.
Type: `PUT`
Url: `http://localhost:8008/_matrix/client/v3/rooms/<room_id>/state/m.room.message/`
Body:
```json
{
  "body": "Test message",
  "msgtype": "m.text",
  "sender": "@<username>:localhost"
}
```

Headers:
```json
authorization: Bearer <access_token>
```

Expected result: `200 OK`
Expected response body:
```json
{
  "event_id": "$bWNtquBz478K_4iAzULGnJ8rqMpD9YmorQZF0s3XNB0"
}
```

# Get messages
Purpose: Confirm that sent messages are processed and saved correctly, as  
Type: `GET`
Url: `http://localhost:8008/_matrix/client/v3/rooms/<room_id>/messages`
Body:
```json
{
  "dir": "f"
}
```

Headers:
```json
authorization: Bearer <access_token>
```

Expected result: `200 OK`
Expected response body:
```json
{
  "chunk": [
    {
      "content": {
        "body": "Yay",
        "msgtype": "m.text",
        "sender": "@<username>:localhost"
      },
      "event_id": "$siu7s6JPlMz2wwPlaHzM8FkLC4lOe1rDmam5j_5APCA",
      "origin_server_ts": 1791204804568,
      "room_id": "!WkeejZBBZhtBrNiwsj:localhost",
      "sender": "@<username>:localhost",
      ...
    }, ...
  ]
} 
```