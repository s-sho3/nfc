# -*- encoding: utf-8 -*-
#http://www.ipa.go.jp/security/publications/nist/documents/SP800-73-J.pdf
#http://eternalwindows.jp/security/scard/scard07.html
#ISO/IEC 7816-4, commands

require 'smartcard'
#smartcard-0.4.11.zip from
##http://rubyforge.org/frs/?group_id=4866&release_id=41490
#gem install echoe --no-ri --no-rdoc
#DEVELOPMENT KIT from
##http://rubyinstaller.org/downloads/
###how to install DevKit
####http://www.qualysite.co.jp/tech-blog/?p=2684
###devkitvars.bat
#VC 2005 C++
##http://download.microsoft.com/download/8/E/8/8E85D539-2255-4CFD-AA97-440AE6C6F44A/vc.iso

context = Smartcard::PCSC::Context.new(Smartcard::PCSC::SCOPE_SYSTEM)
readers = context.list_readers nil
context.cancel

# Use the first reader
reader = readers.first

# Connect to the card
card = Smartcard::PCSC::Card.new(context, reader, Smartcard::PCSC::SHARE_SHARED, Smartcard::PCSC::PROTOCOL_ANY)

# Get the protocol to use
card_status = card.status

# Select applet
aid = [0xA0, 0x00, 0x00, 0x00, 0x62, 0x03, 0x01, 0x0C, 0x06, 0x01]
select_apdu = [0x00, 0xA4, 0x04, 0x00, aid.length, aid].flatten
send_ioreq = {Smartcard::PCSC::PROTOCOL_T0 => Smartcard::PCSC::IOREQUEST_T0,
              Smartcard::PCSC::PROTOCOL_T1 => Smartcard::PCSC::IOREQUEST_T1}[card_status[:protocol]]
recv_ioreq = Smartcard::PCSC::IoRequest.new
response = card.transmit(select_apdu.map {|byte| byte.chr}.join(''), send_ioreq, recv_ioreq)
response_str = (0...response.length).map { |i| ' %02x' % response[i].to_i }.join('')
puts "Answer: #{response_str}\n"

# test APDU
test_apdu = [0, 0, 0, 0]
response = card.transmit(test_apdu.map {|byte| byte.chr}.join(''), send_ioreq, recv_ioreq)
response_str = (0...response.length).map { |i| ' %02x' % response[i].to_i }.join('')
puts "Answer: #{response_str}\n"
response_str = (0...response.length-2).map { |i| '%c' % response[i].to_i }.join('')
puts "Answer: #{response_str}\n"

# Deconnect
card.disconnect Smartcard::PCSC::DISPOSITION_LEAVE
context.release

