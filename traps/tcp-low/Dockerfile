FROM python:3.12-slim

WORKDIR /app
RUN pip install --no-cache-dir PyYAML
COPY trap.py .

ENV HONEYFORGE_PROFILE=/app/profile.yaml
USER 10001:10001
EXPOSE 2222
CMD ["python", "-u", "trap.py"]
