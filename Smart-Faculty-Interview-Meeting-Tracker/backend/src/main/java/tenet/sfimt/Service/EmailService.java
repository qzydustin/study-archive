package tenet.sfimt.Service;


import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.mail.javamail.JavaMailSenderImpl;
import org.springframework.mail.javamail.MimeMessageHelper;
import org.springframework.stereotype.Service;
import org.springframework.web.multipart.MultipartFile;
import tenet.sfimt.Model.Email;

import java.util.Date;

@Service
public class EmailService {

    private Logger logger = LoggerFactory.getLogger(getClass());
    @Autowired
    private JavaMailSenderImpl mailSender;

    public void sendMail(Email mail) {
        try {
            sendMimeMail(mail);
        } catch (Exception e) {
            logger.error("sent fail:", e);
            mail.setStatus("fail");
            mail.setError(e.getMessage());
        }

    }

    private void sendMimeMail(Email mail) {
        try {
            MimeMessageHelper messageHelper = new MimeMessageHelper(mailSender.createMimeMessage(), true);
            mail.setFrom(mail.getFrom());
            messageHelper.setFrom(mail.getFrom());
            messageHelper.setTo(mail.getTo());
            messageHelper.setSubject(mail.getSubject());
            messageHelper.setText(mail.getText());
            if (mail.getCc() != null) {
                messageHelper.setCc(mail.getCc().split(","));
            }
            if (mail.getBcc() != null) {
                messageHelper.setCc(mail.getBcc().split(","));
            }
            if (mail.getMultipartFiles() != null) {
                for (MultipartFile multipartFile : mail.getMultipartFiles()) {
                    messageHelper.addAttachment(multipartFile.getOriginalFilename(), multipartFile);
                }
            }
            if (mail.getSentDate() == null) {
                mail.setSentDate(new Date());
                messageHelper.setSentDate(mail.getSentDate());
            }
            mailSender.send(messageHelper.getMimeMessage());
            mail.setStatus("ok");
            logger.info("sent success：{}->{}", mail.getFrom(), mail.getTo());
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }
}